import time
import json
import random
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Configurações de Rede Interna do Docker
BROKER = "mosquitto"
PORT = 1883
TOPIC = "powerguard/sensores"

INFLUX_URL = "http://influxdb:8086"
INFLUX_TOKEN = "my-super-secret-token-123"
INFLUX_ORG = "cesar"
INFLUX_BUCKET = "telemetria_energia"

# Inicialização e conexões com tratamento de erro
print("Aguardando inicialização dos serviços do ecossistema...")
time.sleep(5)

# Conexão InfluxDB
influx_client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

# Conexão MQTT
mqtt_client = mqtt.Client(client_id="PowerGuard-Core", protocol=mqtt.MQTTv311)
mqtt_client.connect(BROKER, PORT, 60)
mqtt_client.loop_start()

print("🔌 Conectado ao Broker e ao InfluxDB. Monitoramento iniciado.")

try:
    while True:
        corrente = round(random.uniform(3.0, 4.0), 2)
        status = "normal"

        if random.random() < 0.15:
            corrente = round(random.uniform(0.0, 0.1), 2)
            status = "corte"

        # 1. Envio em tempo real para o Frontend (MQTT)
        payload = json.dumps({"corrente": corrente, "status": status})
        mqtt_client.publish(TOPIC, payload)
        print(f"[MQTT Out] Publicado: {payload}")

        # 2. Persistência Histórica no Banco de Dados (InfluxDB)
        ponto = Point("leitura_corrente") \
            .tag("dispositivo", "esp32_sala_infra") \
            .field("corrente_rms", corrente) \
            .field("status_rede", status) \
            .time(time.time_ns(), WritePrecision.NS)
        
        write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=ponto)
        print(f"[DB Influx] Registro persistido com sucesso.")

        time.sleep(2)

except KeyboardInterrupt:
    print("\nEncerrando serviços...")
    mqtt_client.loop_stop()
    influx_client.close()