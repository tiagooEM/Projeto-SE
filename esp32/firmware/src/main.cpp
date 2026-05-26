#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------------- CONFIGURAÇÕES DO SENSOR ----------------
#define SCT_PIN 34
#define ADC_RESOLUTION 4095.0
#define ADC_VREF 3.3
#define NUM_SAMPLES 1000
#define SCT_SCALE 20.0
#define CURRENT_THRESHOLD 0.15

float ADC_OFFSET = 1.65; // Será calibrado no setup

// ---------------- CONFIGURAÇÕES REDE E MQTT --------------
const char* ssid = "uaifai-tiradentes";
const char* password = "bemvindoaocesar";
const char* mqtt_server = "172.26.68.103";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- FILAS DO FREERTOS ----------------------
QueueHandle_t currentQueue;
QueueHandle_t mqttQueue;

typedef struct {
    float current;
    char status[20];
} SensorData;

// ---------------- FUNÇÕES AUXILIARES ---------------------
float readCurrentRMS() {
    double sumSquares = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        int adc = analogRead(SCT_PIN);
        float voltage = (adc * ADC_VREF) / ADC_RESOLUTION;
        float centered = voltage - ADC_OFFSET;
        sumSquares += centered * centered;
        delayMicroseconds(200);
    }
    float rmsVoltage = sqrt(sumSquares / NUM_SAMPLES);
    float current = rmsVoltage * SCT_SCALE;

    // Filtro de ruído: Se a corrente for muito baixa, assume 0
    if (current < CURRENT_THRESHOLD) {
        current = 0;
    }
    return current;
}

// ---------------- TAREFAS (TASKS) ------------------------

void taskReadCurrent(void *pvParameters) {
    SensorData data;
    while (true) {
        data.current = readCurrentRMS();
        xQueueSend(currentQueue, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void taskDetectPower(void *pvParameters) {
    SensorData received;
    while (true) {
        if (xQueueReceive(currentQueue, &received, portMAX_DELAY)) {
            Serial.print("Corrente RMS: ");
            Serial.print(received.current);
            Serial.println(" A");

            if (received.current < CURRENT_THRESHOLD) {
                Serial.println("⚠ POSSÍVEL CORTE DE ENERGIA!");
                strcpy(received.status, "corte");
            } else {
                Serial.println("✅ Energia OK");
                strcpy(received.status, "normal");
            }
            Serial.println("-------------------------");
            xQueueSend(mqttQueue, &received, portMAX_DELAY);
        }
    }
}

void taskMQTT(void *pvParameters) {
    SensorData dataToSend;
    char jsonBuffer[100];

    while (true) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.print("Conectando ao WiFi...");
            WiFi.begin(ssid, password);
            while (WiFi.status() != WL_CONNECTED) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                Serial.print(".");
            }
            Serial.println("\nWiFi Conectado!");
        }

        if (!client.connected()) {
            Serial.print("Conectando ao Broker MQTT...");
            String clientId = "ESP32-PowerGuard-" + String(random(0xffff), HEX);
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

        if (xQueueReceive(mqttQueue, &dataToSend, pdMS_TO_TICKS(100))) {
            snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"corrente\": %.2f, \"status\": \"%s\"}", 
                     dataToSend.current, dataToSend.status);
            
            client.publish("powerguard/sensores", jsonBuffer);
            Serial.print("Publicado no MQTT: ");
            Serial.println(jsonBuffer);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

// ---------------- SETUP E LOOP ---------------------------

void setup() {
    Serial.begin(115200);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    // --- CALIBRAÇÃO AUTOMÁTICA ---
    Serial.println("Calibrando sensor (certifique-se que não há carga)...");
    double totalVoltage = 0;
    for (int i = 0; i < 2000; i++) {
        int adc = analogRead(SCT_PIN);
        totalVoltage += (adc * ADC_VREF) / ADC_RESOLUTION;
        delayMicroseconds(100);
    }
    ADC_OFFSET = totalVoltage / 2000.0;
    Serial.print("Offset Calibrado: ");
    Serial.println(ADC_OFFSET);
    // -----------------------------

    client.setServer(mqtt_server, mqtt_port);

    currentQueue = xQueueCreate(5, sizeof(SensorData));
    mqttQueue = xQueueCreate(5, sizeof(SensorData));

    xTaskCreatePinnedToCore(taskReadCurrent, "ReadCurrent", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskDetectPower, "DetectPower", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskMQTT, "TaskMQTT", 4096, NULL, 1, NULL, 0);
}

void loop() {
}