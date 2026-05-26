# Relatório parcial — Projeto-SE

## Resumo
Projeto de monitoramento de energia usando ESP32. Objetivo: detectar corte/retorno de energia e publicar leituras RMS de corrente via MQTT.

## Arquitetura
ESP32 (sensor de corrente) → Broker MQTT → Backend (subscriber) → Dashboard

## Tópicos MQTT sugeridos
- `casa/sala/corrente/rms` — mensagens JSON com leitura RMS (A)
- `casa/sala/alerta/energia` — mensagens de alerta (corte/retorno)

## Firmware
O firmware encontra-se em `/esp32-esp8266/firmware` e publica leituras periodicamente.

## Testes realizados
- Leitura RMS do sensor implementada (arquivo `main.cpp`).
- Detecção de corrente abaixo do limiar com mensagem serial.

## Próximos passos para validação do checkpoint
- Configurar broker MQTT (Mosquitto) e testar publicação do ESP32.
- Implementar subscriber no backend para armazenar/encaminhar dados.
- Desenvolver dashboard simples para exibir leituras em tempo real.

## Anexos
Adicionar imagens, diagramas e prints na pasta `/docs/images` (criar se necessário).