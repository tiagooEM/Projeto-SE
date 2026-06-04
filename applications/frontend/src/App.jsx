import React, { useEffect, useRef, useState } from 'react';
import mqtt from 'mqtt';

/* =============================================================================
 * PowerGuard — Centro de Operação da Rede
 * -----------------------------------------------------------------------------
 * Painel de monitoramento operacional de corrente elétrica.
 *
 * Fonte de dados:
 *   - O fluxo MQTT real (tópico "powerguard/sensores") alimenta o dispositivo
 *     primário (PG-01). O contrato é o mesmo do simulador: {corrente, status}.
 *   - Os demais pontos são SIMULADOS aqui no front, no mesmo formato, enquanto
 *     o hardware não está pronto.
 *
 * Compatível com o futuro: se o payload passar a incluir "device_id" (ou
 * "dispositivo"), a leitura é roteada para o dispositivo certo — e dispositivos
 * reais desconhecidos são criados automaticamente. Nada precisa mudar aqui.
 * ========================================================================== */

const TOPICO = 'powerguard/sensores';
const BROKER_URL = 'ws://localhost:9002';
const ID_PRIMARIO = 'PG-01';          // dispositivo alimentado pelo MQTT real
const MAX_HISTORICO = 60;             // ~2 min de janela (leitura a cada 2s)
const Y_MAX = 5;                      // escala do eixo Y do gráfico (A)
const LIMIAR_CORTE = 0.5;             // referência visual de corte (A)
const INTERVALO_MS = 2000;            // mesma cadência do simulador.py

// Pontos de monitoramento. Apenas PG-01 é "real" (vem do MQTT); o resto é
// simulado no mesmo formato. `base` = corrente nominal de cada ponto.
const DISPOSITIVOS_INICIAIS = [
  { id: 'PG-01', nome: 'Poste 01 — Sala Infra',      alimentador: 'AL-CENTRO-01', base: 3.5, real: true },
  { id: 'PG-02', nome: 'Poste 02 — Av. Boa Viagem',  alimentador: 'AL-SUL-04',    base: 3.2, real: false },
  { id: 'PG-03', nome: 'Poste 03 — Rua da Aurora',   alimentador: 'AL-CENTRO-02', base: 2.8, real: false },
  { id: 'PG-04', nome: 'Poste 04 — Pina',            alimentador: 'AL-SUL-02',    base: 3.9, real: false },
  { id: 'PG-05', nome: 'Poste 05 — Derby',           alimentador: 'AL-OESTE-01',  base: 3.1, real: false },
  { id: 'PG-06', nome: 'Poste 06 — Casa Forte',      alimentador: 'AL-NORTE-03',  base: 2.6, real: false },
  { id: 'PG-07', nome: 'Poste 07 — Espinheiro',      alimentador: 'AL-NORTE-01',  base: 3.6, real: false },
  { id: 'PG-08', nome: 'Poste 08 — Boa Vista',       alimentador: 'AL-CENTRO-03', base: 3.3, real: false },
];

// Paleta do painel (tema escuro estilo NOC / SCADA)
const C = {
  bg: '#0a1120', painel: '#0f1b2e', painel2: '#13233b', borda: '#203150',
  texto: '#e7eef9', textoFraco: '#8aa0bd', textoFraco2: '#5d728f',
  acento: '#2f81f7',
  ok: '#3fb950', okBg: 'rgba(63,185,80,0.10)',
  alerta: '#e3b341',
  perigo: '#f85149', perigoBg: 'rgba(248,81,73,0.10)',
};

/* ----------------------------- utilidades --------------------------------- */

function leituraRuntime(d) {
  return { ...d, corrente: 0, status: 'normal', history: [], cortes: 0, online: false, ultima: null };
}

// Gera uma leitura simulada no MESMO formato do simulador.py.
function gerarLeitura(base) {
  if (Math.random() < 0.12) {
    return { corrente: +(Math.random() * 0.1).toFixed(2), status: 'corte' };
  }
  const ruido = (Math.random() - 0.5) * 0.5;
  return { corrente: +Math.max(0, base + ruido).toFixed(2), status: 'normal' };
}

function criarDispositivoDesconhecido(id) {
  return leituraRuntime({ id, nome: `Dispositivo ${id}`, alimentador: '—', base: 3.5, real: true });
}

function aplicarLeitura(d, leitura, agora) {
  const entrouEmCorte = d.status !== 'corte' && leitura.status === 'corte';
  const history = [...d.history, { t: agora, corrente: leitura.corrente, status: leitura.status }].slice(-MAX_HISTORICO);
  return {
    ...d,
    corrente: leitura.corrente,
    status: leitura.status,
    history,
    online: true,
    ultima: agora,
    cortes: d.cortes + (entrouEmCorte ? 1 : 0),
  };
}

function horaCurta(ts) {
  return ts ? new Date(ts).toLocaleTimeString('pt-BR') : '—';
}

/* ----------------------------- componentes -------------------------------- */

function Sparkline({ history, corte }) {
  const W = 132, H = 38;
  if (!history || history.length < 2) {
    return <div style={{ width: W, height: H }} />;
  }
  const n = history.length;
  const x = (i) => (W * i) / (n - 1);
  const y = (v) => H - 3 - (H - 6) * (Math.min(v, Y_MAX) / Y_MAX);
  const pts = history.map((h, i) => `${x(i).toFixed(1)},${y(h.corrente).toFixed(1)}`).join(' ');
  const cor = corte ? C.perigo : C.ok;
  return (
    <svg width={W} height={H} style={{ display: 'block' }}>
      <polyline points={pts} fill="none" stroke={cor} strokeWidth="1.6"
        strokeLinejoin="round" strokeLinecap="round" />
    </svg>
  );
}

function Grafico({ history, corte }) {
  const W = 760, H = 260, pad = 38;
  const cor = corte ? C.perigo : C.acento;
  const x = (i, n) => pad + (W - 2 * pad) * (n <= 1 ? 0 : i / (n - 1));
  const y = (v) => H - pad - (H - 2 * pad) * (Math.min(v, Y_MAX) / Y_MAX);
  const n = history.length;

  const linhas = [];
  for (let a = 0; a <= Y_MAX; a++) {
    const gy = y(a);
    linhas.push(
      <g key={a}>
        <line x1={pad} y1={gy} x2={W - pad} y2={gy} stroke={C.borda} strokeWidth="1" />
        <text x={pad - 8} y={gy + 4} textAnchor="end" fontSize="11" fill={C.textoFraco2}>{a}A</text>
      </g>
    );
  }

  const pts = history.map((h, i) => `${x(i, n).toFixed(1)},${y(h.corrente).toFixed(1)}`);
  const linha = pts.join(' ');
  const area = n > 1
    ? `M ${pts[0]} L ${pts.join(' L ')} L ${x(n - 1, n).toFixed(1)},${H - pad} L ${pad},${H - pad} Z`
    : '';

  return (
    <svg width="100%" viewBox={`0 0 ${W} ${H}`} preserveAspectRatio="none" style={{ display: 'block' }}>
      <defs>
        <linearGradient id="pg-area" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stopColor={cor} stopOpacity="0.28" />
          <stop offset="100%" stopColor={cor} stopOpacity="0" />
        </linearGradient>
      </defs>
      {linhas}
      {/* limiar de corte */}
      <line x1={pad} y1={y(LIMIAR_CORTE)} x2={W - pad} y2={y(LIMIAR_CORTE)}
        stroke={C.perigo} strokeWidth="1" strokeDasharray="5 4" opacity="0.6" />
      {n > 1 && <path d={area} fill="url(#pg-area)" />}
      {n > 1 && (
        <polyline points={linha} fill="none" stroke={cor} strokeWidth="2.2"
          strokeLinejoin="round" strokeLinecap="round" />
      )}
      {n > 0 && (
        <circle cx={x(n - 1, n)} cy={y(history[n - 1].corrente)} r="4" fill={cor}
          stroke={C.painel} strokeWidth="2" />
      )}
      {n === 0 && (
        <text x={W / 2} y={H / 2} textAnchor="middle" fontSize="14" fill={C.textoFraco2}>
          Aguardando leituras…
        </text>
      )}
    </svg>
  );
}

function Kpi({ label, valor, unidade, cor, destaque }) {
  return (
    <div style={{
      flex: 1, minWidth: 140, background: C.painel2, border: `1px solid ${C.borda}`,
      borderRadius: 10, padding: '14px 16px',
    }}>
      <div style={{ fontSize: 11, letterSpacing: 0.6, textTransform: 'uppercase', color: C.textoFraco }}>
        {label}
      </div>
      <div style={{ marginTop: 6, fontSize: 26, fontWeight: 700, color: cor || C.texto, lineHeight: 1 }}>
        {valor}
        {unidade && <span style={{ fontSize: 13, fontWeight: 500, color: C.textoFraco, marginLeft: 4 }}>{unidade}</span>}
      </div>
      {destaque && <div style={{ marginTop: 6, fontSize: 11, color: C.textoFraco2 }}>{destaque}</div>}
    </div>
  );
}

function CardDispositivo({ d, selecionado, onClick }) {
  const corte = d.status === 'corte';
  return (
    <button
      onClick={onClick}
      className={corte ? 'pg-pulse' : ''}
      style={{
        textAlign: 'left', cursor: 'pointer', width: '100%',
        background: selecionado ? C.painel2 : C.painel,
        border: `1px solid ${selecionado ? C.acento : corte ? C.perigo : C.borda}`,
        borderRadius: 10, padding: 12, color: C.texto, font: 'inherit',
        display: 'flex', flexDirection: 'column', gap: 8,
      }}
    >
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start', gap: 8 }}>
        <div>
          <div style={{ fontSize: 13, fontWeight: 600 }}>{d.nome}</div>
          <div style={{ fontSize: 11, color: C.textoFraco }}>
            {d.id} · {d.alimentador}{d.real ? ' · sensor real' : ''}
          </div>
        </div>
        <span style={{
          fontSize: 10, fontWeight: 700, letterSpacing: 0.5, padding: '3px 8px', borderRadius: 20,
          color: corte ? C.perigo : C.ok, background: corte ? C.perigoBg : C.okBg,
          border: `1px solid ${corte ? C.perigo : C.ok}`, whiteSpace: 'nowrap',
        }}>
          {corte ? 'CORTE' : 'NORMAL'}
        </span>
      </div>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-end' }}>
        <div style={{ fontSize: 22, fontWeight: 700, color: corte ? C.perigo : C.texto }}>
          {d.corrente.toFixed(2)}<span style={{ fontSize: 12, color: C.textoFraco, marginLeft: 3 }}>A</span>
        </div>
        <Sparkline history={d.history} corte={corte} />
      </div>
      <div style={{ fontSize: 10, color: C.textoFraco2 }}>
        {d.cortes} corte(s) · atualizado {horaCurta(d.ultima)}
      </div>
    </button>
  );
}

/* ------------------------------- app -------------------------------------- */

function App() {
  const [dispositivos, setDispositivos] = useState(() => DISPOSITIVOS_INICIAIS.map(leituraRuntime));
  const [eventos, setEventos] = useState([]);
  const [conectado, setConectado] = useState(false);
  const [selecionado, setSelecionado] = useState(ID_PRIMARIO);
  const [agora, setAgora] = useState(() => new Date());

  // Refs para evitar closures obsoletas dentro dos handlers/intervalos.
  const dispRef = useRef(dispositivos);
  const conectadoRef = useRef(conectado);
  useEffect(() => { dispRef.current = dispositivos; }, [dispositivos]);
  useEffect(() => { conectadoRef.current = conectado; }, [conectado]);

  // Registra uma leitura (real ou simulada) para um dispositivo.
  function registrarLeitura(deviceId, leitura) {
    if (!Number.isFinite(leitura.corrente)) return;
    const ts = Date.now();
    const atual = dispRef.current.find((d) => d.id === deviceId);

    if (atual) {
      const entrouEmCorte = atual.status !== 'corte' && leitura.status === 'corte';
      const saiuDeCorte = atual.status === 'corte' && leitura.status !== 'corte';
      if (entrouEmCorte || saiuDeCorte) {
        setEventos((prev) => [
          { id: `${deviceId}-${ts}`, t: ts, deviceId, nome: atual.nome, tipo: entrouEmCorte ? 'corte' : 'restabelecido' },
          ...prev,
        ].slice(0, 40));
      }
    }

    setDispositivos((prev) => {
      const base = prev.some((d) => d.id === deviceId) ? prev : [...prev, criarDispositivoDesconhecido(deviceId)];
      return base.map((d) => (d.id === deviceId ? aplicarLeitura(d, leitura, ts) : d));
    });
  }
  const registrarRef = useRef(registrarLeitura);
  registrarRef.current = registrarLeitura;

  // Conexão MQTT — alimenta o dispositivo primário (ou o device_id informado).
  useEffect(() => {
    const client = mqtt.connect(BROKER_URL);

    client.on('connect', () => {
      setConectado(true);
      client.subscribe(TOPICO);
    });
    client.on('message', (_topic, message) => {
      try {
        const payload = JSON.parse(message.toString());
        const deviceId = payload.device_id || payload.dispositivo || ID_PRIMARIO;
        registrarRef.current(deviceId, { corrente: Number(payload.corrente), status: payload.status });
      } catch (e) {
        console.error('Payload MQTT inválido:', e);
      }
    });
    client.on('error', (err) => console.error('Erro MQTT:', err));
    client.on('close', () => setConectado(false));
    client.on('offline', () => setConectado(false));

    return () => client.end();
  }, []);

  // Simulação: alimenta os pontos simulados — e também o primário enquanto o
  // broker estiver offline, para o painel nunca ficar parado numa demo.
  useEffect(() => {
    const intervalo = setInterval(() => {
      dispRef.current.forEach((d) => {
        if (!d.real || !conectadoRef.current) {
          registrarRef.current(d.id, gerarLeitura(d.base));
        }
      });
    }, INTERVALO_MS);
    return () => clearInterval(intervalo);
  }, []);

  // Relógio do cabeçalho.
  useEffect(() => {
    const t = setInterval(() => setAgora(new Date()), 1000);
    return () => clearInterval(t);
  }, []);

  /* ----- métricas derivadas ----- */
  const emCorte = dispositivos.filter((d) => d.status === 'corte');
  const normais = dispositivos.filter((d) => d.status !== 'corte');
  const totalCortes = dispositivos.reduce((s, d) => s + d.cortes, 0);
  const correnteMedia = normais.length
    ? normais.reduce((s, d) => s + d.corrente, 0) / normais.length
    : 0;
  const cargaTotal = dispositivos.reduce((s, d) => s + d.corrente, 0);
  const online = dispositivos.filter((d) => d.online).length;

  const dispSel = dispositivos.find((d) => d.id === selecionado) || dispositivos[0];
  const aoVivo = conectado;

  /* ----- estilos auxiliares ----- */
  const painel = { background: C.painel, border: `1px solid ${C.borda}`, borderRadius: 12 };
  const tituloPainel = {
    margin: 0, padding: '12px 16px', fontSize: 12, fontWeight: 700, letterSpacing: 0.8,
    textTransform: 'uppercase', color: C.textoFraco, borderBottom: `1px solid ${C.borda}`,
  };

  return (
    <div style={{
      minHeight: '100vh', background: C.bg, color: C.texto,
      fontFamily: "'Segoe UI', Roboto, Helvetica, Arial, sans-serif",
    }}>
      <style>{`
        * { box-sizing: border-box; }
        body { margin: 0; }
        @keyframes pgPulse {
          0%   { box-shadow: 0 0 0 0 rgba(248,81,73,0.45); }
          70%  { box-shadow: 0 0 0 8px rgba(248,81,73,0); }
          100% { box-shadow: 0 0 0 0 rgba(248,81,73,0); }
        }
        .pg-pulse { animation: pgPulse 1.6s infinite; }
        .pg-scroll::-webkit-scrollbar { width: 8px; }
        .pg-scroll::-webkit-scrollbar-thumb { background: ${C.borda}; border-radius: 8px; }
      `}</style>

      {/* ---------------------------- cabeçalho ---------------------------- */}
      <header style={{
        display: 'flex', alignItems: 'center', justifyContent: 'space-between',
        padding: '14px 24px', background: C.painel, borderBottom: `1px solid ${C.borda}`,
        position: 'sticky', top: 0, zIndex: 10,
      }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: 14 }}>
          <div style={{
            width: 42, height: 42, borderRadius: 10, display: 'grid', placeItems: 'center',
            background: 'linear-gradient(135deg,#2f81f7,#0a4fb0)', fontSize: 22,
          }}>⚡</div>
          <div>
            <div style={{ fontSize: 19, fontWeight: 700, letterSpacing: 0.3 }}>
              PowerGuard <span style={{ color: C.textoFraco, fontWeight: 400, fontSize: 14 }}>| Centro de Operação da Rede</span>
            </div>
            <div style={{ fontSize: 11, color: C.textoFraco2 }}>
              Monitoramento de corrente elétrica em tempo real
            </div>
          </div>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', gap: 16 }}>
          <div style={{ textAlign: 'right' }}>
            <div style={{ fontSize: 16, fontWeight: 600, fontVariantNumeric: 'tabular-nums' }}>
              {agora.toLocaleTimeString('pt-BR')}
            </div>
            <div style={{ fontSize: 11, color: C.textoFraco }}>
              {agora.toLocaleDateString('pt-BR', { weekday: 'short', day: '2-digit', month: 'short', year: 'numeric' })}
            </div>
          </div>
          <span style={{
            display: 'inline-flex', alignItems: 'center', gap: 7, padding: '7px 12px', borderRadius: 20,
            fontSize: 12, fontWeight: 700, letterSpacing: 0.4,
            color: aoVivo ? C.ok : C.alerta,
            background: aoVivo ? C.okBg : 'rgba(227,179,65,0.12)',
            border: `1px solid ${aoVivo ? C.ok : C.alerta}`,
          }}>
            <span style={{
              width: 8, height: 8, borderRadius: '50%', background: aoVivo ? C.ok : C.alerta,
            }} className="pg-pulse" />
            {aoVivo ? 'AO VIVO · MQTT' : 'MODO SIMULAÇÃO'}
          </span>
        </div>
      </header>

      {/* ------------------------------ KPIs ------------------------------- */}
      <div style={{ display: 'flex', gap: 14, flexWrap: 'wrap', padding: '18px 24px 0' }}>
        <Kpi label="Pontos monitorados" valor={dispositivos.length} destaque={`${online} online`} />
        <Kpi label="Em corte agora" valor={emCorte.length}
          cor={emCorte.length ? C.perigo : C.ok}
          destaque={emCorte.length ? 'requer atenção' : 'rede estável'} />
        <Kpi label="Cortes na sessão" valor={totalCortes} cor={C.alerta} destaque="eventos acumulados" />
        <Kpi label="Corrente média" valor={correnteMedia.toFixed(2)} unidade="A" destaque="pontos normais" />
        <Kpi label="Carga total" valor={cargaTotal.toFixed(1)} unidade="A" destaque="soma instantânea" />
      </div>

      {/* ------------------------- grade principal ------------------------- */}
      <div style={{
        display: 'grid', gridTemplateColumns: 'minmax(300px, 360px) 1fr minmax(260px, 320px)',
        gap: 16, padding: 24, alignItems: 'start',
      }}>
        {/* coluna 1 — dispositivos */}
        <section style={painel}>
          <h2 style={tituloPainel}>Pontos de monitoramento</h2>
          <div className="pg-scroll" style={{
            display: 'flex', flexDirection: 'column', gap: 10, padding: 12,
            maxHeight: '70vh', overflowY: 'auto',
          }}>
            {dispositivos.map((d) => (
              <CardDispositivo key={d.id} d={d} selecionado={d.id === selecionado}
                onClick={() => setSelecionado(d.id)} />
            ))}
          </div>
        </section>

        {/* coluna 2 — gráfico do dispositivo selecionado */}
        <section style={painel}>
          <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', borderBottom: `1px solid ${C.borda}` }}>
            <h2 style={{ ...tituloPainel, borderBottom: 'none' }}>
              Corrente RMS · {dispSel?.nome}
            </h2>
            <div style={{ padding: '0 16px', fontSize: 12, color: C.textoFraco }}>
              {dispSel?.id} · {dispSel?.alimentador}
            </div>
          </div>

          <div style={{ padding: 16 }}>
            <div style={{ display: 'flex', alignItems: 'baseline', gap: 12, marginBottom: 8 }}>
              <div style={{
                fontSize: 44, fontWeight: 700, lineHeight: 1,
                color: dispSel?.status === 'corte' ? C.perigo : C.texto,
              }}>
                {dispSel ? dispSel.corrente.toFixed(2) : '0.00'}
                <span style={{ fontSize: 18, color: C.textoFraco, marginLeft: 6 }}>A</span>
              </div>
              <span style={{
                fontSize: 12, fontWeight: 700, padding: '5px 12px', borderRadius: 20,
                color: dispSel?.status === 'corte' ? C.perigo : C.ok,
                background: dispSel?.status === 'corte' ? C.perigoBg : C.okBg,
                border: `1px solid ${dispSel?.status === 'corte' ? C.perigo : C.ok}`,
              }}>
                {dispSel?.status === 'corte' ? '⚠ CORTE DETECTADO' : '✓ ENERGIA NORMAL'}
              </span>
            </div>
            <Grafico history={dispSel?.history || []} corte={dispSel?.status === 'corte'} />
            <div style={{ display: 'flex', justifyContent: 'space-between', marginTop: 8, fontSize: 11, color: C.textoFraco2 }}>
              <span>Janela: últimas {MAX_HISTORICO} leituras (~{Math.round(MAX_HISTORICO * INTERVALO_MS / 1000 / 60)} min)</span>
              <span style={{ color: C.perigo }}>— — limiar de corte ({LIMIAR_CORTE.toFixed(1)} A)</span>
            </div>
          </div>
        </section>

        {/* coluna 3 — alarmes / eventos */}
        <section style={painel}>
          <h2 style={tituloPainel}>
            Registro de eventos {eventos.length > 0 && (
              <span style={{ color: C.perigo }}>· {eventos.length}</span>
            )}
          </h2>
          <div className="pg-scroll" style={{ maxHeight: '70vh', overflowY: 'auto', padding: 12 }}>
            {eventos.length === 0 && (
              <div style={{ padding: 20, textAlign: 'center', fontSize: 12, color: C.textoFraco2 }}>
                Nenhum evento registrado. A rede está estável.
              </div>
            )}
            {eventos.map((e) => {
              const corte = e.tipo === 'corte';
              return (
                <div key={e.id} style={{
                  display: 'flex', gap: 10, padding: '10px 8px', borderBottom: `1px solid ${C.borda}`,
                }}>
                  <div style={{
                    width: 6, borderRadius: 6, background: corte ? C.perigo : C.ok, flexShrink: 0,
                  }} />
                  <div style={{ flex: 1 }}>
                    <div style={{ fontSize: 12, fontWeight: 600, color: corte ? C.perigo : C.ok }}>
                      {corte ? 'Corte de energia' : 'Energia restabelecida'}
                    </div>
                    <div style={{ fontSize: 11, color: C.textoFraco }}>{e.nome}</div>
                  </div>
                  <div style={{ fontSize: 11, color: C.textoFraco2, whiteSpace: 'nowrap' }}>
                    {horaCurta(e.t)}
                  </div>
                </div>
              );
            })}
          </div>
        </section>
      </div>

      <footer style={{ padding: '0 24px 24px', fontSize: 11, color: C.textoFraco2 }}>
        PowerGuard · Projeto IoT — broker MQTT {BROKER_URL} · tópico {TOPICO}
        {!aoVivo && ' · exibindo dados simulados enquanto o broker está offline'}
      </footer>
    </div>
  );
}

export default App;
