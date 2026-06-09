#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ---------------- CONFIGURAÇÕES DO SENSOR E LED ----------------
const int sensorPin = 34; 
const int ledPin = 2;     // Pino do LED

// ---------------- CONFIGURAÇÕES REDE E MQTT --------------------
const char* ssid = "uaifai-tiradentes";
const char* password = "bemvindoaocesar";
const char* mqtt_server = "172.26.68.103";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- FILAS DO FREERTOS ----------------------------
QueueHandle_t sensorQueue;
QueueHandle_t mqttQueue;

// Estrutura simplificada para o dado do sensor
typedef struct {
    int leitura;
    char status[20];
} SensorData;

// ---------------- TAREFAS (TASKS) ------------------------------

// Tarefa 1: Apenas lê o sensor direto e joga na fila
void taskReadSensor(void *pvParameters) {
    SensorData data;
    while (true) {
        data.leitura = analogRead(sensorPin);
        
        // Envia para a próxima tarefa processar
        xQueueSend(sensorQueue, &data, portMAX_DELAY);
        
        // Faz a leitura a cada 500ms para não floodar o broker MQTT
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

// Tarefa 2: Avalia o valor, liga/desliga o LED e prepara para o MQTT
void taskProcessData(void *pvParameters) {
    SensorData received;
    while (true) {
        if (xQueueReceive(sensorQueue, &received, portMAX_DELAY)) {
            
            Serial.print("Leitura ADC: ");
            Serial.println(received.leitura);

            // Lógica simples que você pediu (adapte o valor 500 se precisar)
            if (received.leitura >= 500) {
                digitalWrite(ledPin, HIGH); // Liga LED
                strcpy(received.status, "ativo");
            } else {
                digitalWrite(ledPin, LOW);  // Apaga LED
                strcpy(received.status, "inativo");
            }

            Serial.println("-------------------------");
            
            // Envia o pacote pronto para a tarefa do MQTT publicar
            xQueueSend(mqttQueue, &received, portMAX_DELAY);
        }
    }
}

// Tarefa 3: Cuida exclusivamente do WiFi e do envio para o Mosquitto
void taskMQTT(void *pvParameters) {
    SensorData dataToSend;
    char jsonBuffer[100];

    while (true) {
        // Mantém o WiFi Conectado
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print("Conectando ao WiFi...");
            WiFi.begin(ssid, password);
            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                Serial.print(".");
            }
            Serial.println("\nWiFi Conectado!");
        }

        // Mantém o Mosquitto Conectado
        if (!client.connected()) {
            Serial.print("Conectando ao Mosquitto...");
            String clientId = "ESP32-Sensor-" + String(random(0xffff), HEX);
            if (client.connect(clientId.c_str())) {
                Serial.println(" Conectado!");
            } else {
                Serial.print(" Falha. Código: ");
                Serial.println(client.state());
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue; 
            }
        }
        
        client.loop();

        // Se tem dado processado na fila, publica no MQTT
        if (xQueueReceive(mqttQueue, &dataToSend, pdMS_TO_TICKS(100))) {
            // Monta o JSON com o valor lido e o status do LED
            snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"leitura_adc\": %d, \"status\": \"%s\"}", 
                     dataToSend.leitura, dataToSend.status);
            
            // Publica no tópico
            client.publish("powerguard/sensores", jsonBuffer);
            Serial.print("Publicado no MQTT: ");
            Serial.println(jsonBuffer);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// ---------------- SETUP E LOOP ---------------------------------

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);

    // Configura o broker Mosquitto
    client.setServer(mqtt_server, mqtt_port);

    // Cria as filas de comunicação entre as tarefas
    sensorQueue = xQueueCreate(5, sizeof(SensorData));
    mqttQueue = xQueueCreate(5, sizeof(SensorData));

    // Inicia as tarefas (distribuídas nos núcleos do ESP32 para não travar o WiFi)
    xTaskCreatePinnedToCore(taskReadSensor, "ReadSensor", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskProcessData, "ProcessData", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskMQTT, "TaskMQTT", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // Loop vazio, o FreeRTOS faz o trabalho
}