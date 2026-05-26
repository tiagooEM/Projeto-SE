# Contribuindo

## Como contribuir

1. **Clone o repositório** e crie uma branch com seu nome ou feature:
   ```bash
   git checkout -b feat/sua-feature
   git checkout -b fix/seu-bugfix
   ```

2. **Commits claros** — use mensagens em português ou inglês:
   ```bash
   git commit -m "feat: adicionar leitura RMS do sensor"
   git commit -m "fix: corrigir conexão MQTT intermitente"
   git commit -m "docs: atualizar tópicos MQTT"
   ```

3. **Push e abra um PR** para revisão antes de fazer merge em `main`.

## Estrutura de branches sugerida
- `main` — código estável e pronto para entrega
- `develop` — integração de features (opcional)
- `feat/*` — novas funcionalidades
- `fix/*` — correção de bugs
- `docs/*` — documentação

## Testes mínimos antes de commit
- Firmware: verifique leitura do sensor com `Serial.println()`.
- Backend: teste conexão MQTT e publicação em `http://localhost:5000`.
- Frontend: abra o navegador e confirme atualização dos gráficos.

## Recomendações
- Commits pequenos e frequentes são melhores que poucos commits grandes.
- Cada integrante faz seus próprios commits (não squash em excesso).
- Documente mudanças em `.env.example` ou `docs/` se adicionar variáveis/tópicos.

Dúvidas? Abra uma issue ou entre em contato com o grupo.
