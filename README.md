# Projeto-SE

**Nome do projeto:** Projeto-SE

**Tema escolhido:** Monitoramento de Energia (MNR)

**Objetivo do sistema:** Detectar disponibilidade de energia e publicar leituras de corrente via MQTT para backend e dashboard.

**Integrantes:** Felipe Santos, Luiz Felipe Arruda, Marcelo Bresani, Silvio Fittipaldi, Felipe, Tiago  Emilio, Rodrigo Nunes

**Tecnologias utilizadas:** ESP32, PlatformIO, FreeRTOS, MQTT (Mosquitto sugerido), Backend (ex.: Flask/Node), Frontend (dashboard web)

**Arquitetura do projeto:**
ESP32 → Broker MQTT → Backend → Dashboard

**Fluxo de comunicação:**
- ESP32 lê sensor de corrente e publica leituras RMS em tópico MQTT.
- Backend assina o tópico MQTT, processa e persiste (opcional).
- Dashboard consome via WebSocket/HTTP do backend ou direto do broker para exibir dados em tempo real.

**Como executar:**

1. **Broker MQTT (Mosquitto + Node-RED):**
   ```bash
   docker compose up -d
   ```
   - Mosquitto: `localhost:1883`
   - Node-RED: `http://localhost:1880`

2. **Backend (Flask + Socket.IO + MQTT subscriber):**
   ```bash
   cd applications/backend
   python -m venv .venv
   .\.venv\Scripts\activate  # Windows
   # source .venv/bin/activate  # Linux/Mac
   pip install -r requirements.txt
   python app.py
   ```
   - Dashboard: `http://localhost:5000`

3. **Firmware ESP32 (PlatformIO):**
   - Abra `esp32-esp8266/firmware` no VS Code com extensão PlatformIO.
   - Edite credenciais WiFi em `src/main.cpp`: `ssid`, `password`, `mqtt_server`.
   - Faça upload: `Ctrl+Alt+U` (ou menu PlatformIO → Upload).

4. **Testar fluxo MQTT (publicar mensagem de teste):**
   ```bash
   docker exec powerguard-mosquitto mosquitto_pub -h localhost -t powerguard/sensores -m '{"corrente": 5.23, "status": "normal"}'
   ```

**Prints/Imagens:** adicionar em `/docs` (GIFs e imagens recomendados).

**Organização das pastas:**
- `/docs` — documentação e relatório parcial
- `/applications/backend` — código backend (server, subscribers)
- `/applications/frontend` — dashboard / frontend
- `/esp32-esp8266` — firmware ESP32/ESP8266 (PlatformIO)
- `/schematics` — diagramas do circuito

**Status atual do projeto:**
- ✅ Firmware funcional para leitura RMS, detecção de corte e publicação MQTT.
- ✅ Backend funcional: subscriber MQTT + Socket.IO para frontend.
- ✅ Frontend protótipo: dashboard em tempo real com Chart.js.
- ✅ Broker Mosquitto rodando em Docker.
- ⏳ Testes em ESP32 real (aguardando flashing com credenciais Wi-Fi).

**Próximos passos:**
- Adicionar imagens/diagramas em `/schematics` e `/docs`.
- Validar fluxo end-to-end com ESP32 real conectado à rede.
- (Opcional) Melhorar persistência de dados e adicionar banco de dados.

**Contribuindo:**
Ver [CONTRIBUTING.md](CONTRIBUTING.md) para orientações de commits e branches.

---

Atualizado para checkpoint (26/05). Todos os componentes mínimos estão funcionales. Próximos passos: completar documentação e validar com hardware real.