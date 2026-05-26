# Tópicos MQTT e payloads

Tópico principal usado pelo firmware:
- `powerguard/sensores` — mensagens JSON com leitura de corrente e status.

Exemplo de payload publicado pelo ESP32:
```json
{"corrente": 0.45, "status": "normal"}
```

Outros tópicos sugeridos:
- `powerguard/alertas` — mensagens de alerta (corte/retorno)

Observação: configure o broker MQTT em `docker-compose.yml` ou oponha um broker público e ajuste `mqtt_server` no firmware ou use variáveis de ambiente em `.env`.