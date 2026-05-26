import os
import json
from threading import Thread
from dotenv import load_dotenv
from flask import Flask
from flask_socketio import SocketIO
import paho.mqtt.client as mqtt

load_dotenv()

MQTT_HOST = os.getenv('MQTT_HOST', 'localhost')
MQTT_PORT = int(os.getenv('MQTT_PORT', '1883'))
MQTT_TOPIC = os.getenv('MQTT_TOPIC', 'powerguard/sensores')

app = Flask(__name__, static_folder='../frontend', static_url_path='/')
socketio = SocketIO(app, cors_allowed_origins='*', async_mode='eventlet')

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    print('MQTT conectado, código:', rc)
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    try:
        data = json.loads(payload)
    except Exception:
        data = {'raw': payload}
    print('Mensagem MQTT:', data)
    socketio.emit('sensor_data', data)

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

def mqtt_loop():
    mqtt_client.connect(MQTT_HOST, MQTT_PORT)
    mqtt_client.loop_forever()

@app.route('/')
def index():
    return app.send_static_file('index.html')

if __name__ == '__main__':
    # start mqtt in background thread
    t = Thread(target=mqtt_loop, daemon=True)
    t.start()
    socketio.run(app, host=os.getenv('FLASK_HOST','0.0.0.0'), port=int(os.getenv('FLASK_PORT', 5000)))
