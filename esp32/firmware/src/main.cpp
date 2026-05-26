#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------------- CONFIGURAÇÕES DO SENSOR ----------------
#define SCT_PIN 34
#define ADC_RESOLUTION 4095.0
#define ADC_VREF 3.3
#define ADC_OFFSET 1.65
#define NUM_SAMPLES 1000
#define SCT_SCALE 20.0
#define CURRENT_THRESHOLD 0.15

// ---------------- CONFIGURAÇÕES REDE E MQTT --------------
const char* ssid = "SEU_NOME_DA_REDE_WIFI";
const char* password = "SUA_SENHA_WIFI";
const char* mqtt_server = "IP_DO_SEU_BROKER_OU_RASPBERRY";
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
    return abs(current);
}

// ---------------- TAREFAS (TASKS) ------------------------

// Task 1: Lê o sensor continuamente (AGORA NO CORE 1)
void taskReadCurrent(void *pvParameters) {
    SensorData data;
    while (true) {
        data.current = readCurrentRMS();
        xQueueSend(currentQueue, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task 2: Analisa os dados (AGORA NO CORE 1)
void taskDetectPower(void *pvParameters) {
    SensorData received;
    while (true) {
        if (xQueueReceive(currentQueue, &received, portMAX_DELAY)) {
            Serial.print("Corrente RMS: ");
            Serial.print(received.current);
            Serial.println(" A");

            // DETECÇÃO DE STATUS
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

// Task 3: Gerencia o Wi-Fi e publica no MQTT (AGORA NO CORE 0)
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

    client.setServer(mqtt_server, mqtt_port);

    currentQueue = xQueueCreate(5, sizeof(SensorData));
    mqttQueue = xQueueCreate(5, sizeof(SensorData));

    // Inicialização das Tasks com a alocação de núcleos invertida
    xTaskCreatePinnedToCore(taskReadCurrent, "ReadCurrent", 4096, NULL, 1, NULL, 1); // Core 1 (APP_CPU)
    xTaskCreatePinnedToCore(taskDetectPower, "DetectPower", 4096, NULL, 1, NULL, 1); // Core 1 (APP_CPU)
    xTaskCreatePinnedToCore(taskMQTT, "TaskMQTT", 4096, NULL, 1, NULL, 0);           // Core 0 (PRO_CPU)
}

void loop() {
}