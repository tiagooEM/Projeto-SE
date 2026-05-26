import time
import json
import random
import paho.mqtt.client as mqtt

# Aponta para o nome do contêiner do broker no Docker Compose
BROKER = "mosquitto" 
PORT = 1883
TOPIC = "powerguard/sensores"

client = mqtt.Client(client_id="PowerGuard-Python", protocol=mqtt.MQTTv311)

print("Tentando conectar ao broker Mosquitto...")
while True:
    try:
        client.connect(BROKER, PORT, 60)
        break
    except Exception as e:
        print("Aguardando o broker iniciar...")
        time.sleep(2)

client.loop_start()
print("🔌 Simulador PowerGuard iniciado. Enviando dados...")

try:
    while True:
        corrente = round(random.uniform(3.0, 4.0), 2)
        status = "normal"

        if random.random() < 0.15:
            corrente = round(random.uniform(0.0, 0.1), 2)
            status = "corte"

        payload = json.dumps({"corrente": corrente, "status": status})
        
        client.publish(TOPIC, payload)
        print(f"Enviado: {payload}")
        
        time.sleep(2)

except KeyboardInterrupt:
    print("\nSimulação encerrada.")
    client.loop_stop()
    client.disconnect()