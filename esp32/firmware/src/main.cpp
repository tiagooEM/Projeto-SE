#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ---------------- CONFIGURAÇÕES DO SENSOR E LED ----------------
const int sensorPin = 34; 
const int ledPin = 2;

// ---------------- CONFIGURAÇÕES REDE E MQTT --------------------
const char* ssid = "uaifai-tiradentes";
const char* password = "bemvindoaocesar";
const char* mqtt_server = "172.26.68.103";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- FILAS E VARIÁVEIS GLOBAIS --------------------
QueueHandle_t sensorQueue;
QueueHandle_t mqttQueue;

volatile bool aparelhoLigado = false;

typedef struct {
    int leitura; 
    char status[20];
} SensorData;

const int TAMANHO_MEDIA = 10;
int bufferLeituras[TAMANHO_MEDIA] = {0};
int indiceMedia = 0;
int contadorMedia = 0;

// ---------------- TAREFAS (TASKS) ------------------------------

// Tarefa 1: Lê o sensor capturando leitura instantânea de corrente
void taskReadSensor(void *pvParameters) {
    SensorData data;
    while (true) {
        // Calcula média de 10 leituras para suavizar ruído
        int soma = 0;
        for (int i = 0; i < 10; i++) {
            soma += analogRead(sensorPin);
            delayMicroseconds(100);
        }
        
        data.leitura = soma / 10;
        
        // PENTE FINO 1: Em vez de portMAX_DELAY, esperamos no máximo 10ms. 
        // Se a fila estiver cheia (ex: MQTT travado), descartamos a leitura para não travar o ESP32.
        xQueueSend(sensorQueue, &data, pdMS_TO_TICKS(10));
        
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

// Tarefa 2: Calcula a média e aplica Histerese
void taskCalcularMedia(void *pvParameters) {
    SensorData received;
    while (true) {
        // Aguarda dados da fila do sensor
        if (xQueueReceive(sensorQueue, &received, portMAX_DELAY)) {
            
            bufferLeituras[indiceMedia] = received.leitura;
            indiceMedia = (indiceMedia + 1) % TAMANHO_MEDIA;
            contadorMedia++;
            
            if (contadorMedia >= TAMANHO_MEDIA) {
                int soma = 0;
                for (int i = 0; i < TAMANHO_MEDIA; i++) {
                    soma += bufferLeituras[i];
                }
                int mediaVariacao = soma / TAMANHO_MEDIA;
                
                // PENTE FINO 2: HISTERESE (Evita o efeito "pisca-pisca" no status)
                // Se está ligado, precisa cair muito para considerarmos desligado (ex: abaixo de 300)
                // Se está desligado, precisa subir muito para considerarmos ligado (ex: acima de 600)
                if (aparelhoLigado) {
                    if (mediaVariacao < 300) { 
                        aparelhoLigado = false;
                    }
                } else {
                    if (mediaVariacao > 600) {
                        aparelhoLigado = true;
                    }
                }

                if (aparelhoLigado) {
                    digitalWrite(ledPin, HIGH); 
                    strcpy(received.status, "ativo");
                } else {
                    digitalWrite(ledPin, LOW);  
                    strcpy(received.status, "inativo");
                }

                received.leitura = mediaVariacao;
                
                // PENTE FINO 1 (Continuação): Envia para a fila do MQTT sem bloquear o sistema
                xQueueSend(mqttQueue, &received, pdMS_TO_TICKS(10));
                
                // Reset seguro do contador, mantendo o buffer circular girando corretamente
                if (contadorMedia > TAMANHO_MEDIA * 2) {
                    contadorMedia = TAMANHO_MEDIA; 
                }
            }
        }
    }
}

// Tarefa 3: Print Independente
void taskPrintStatus(void *pvParameters) {
    while (true) {
        if (aparelhoLigado) {
            Serial.println("Status: LIGADO");
        } else {
            Serial.println("Status: DESLIGADO");
        }
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}

// Tarefa 4: Gerencia Conexão e MQTT
void taskMQTT(void *pvParameters) {
    SensorData dataToSend;
    char jsonBuffer[100];

    while (true) {
        // Tenta manter o WiFi vivo
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            WiFi.begin(ssid, password);
            // Aguarda conectar, mas com um limite para não travar infinitamente
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 10) {
                vTaskDelay(pdMS_TO_TICKS(1000));
                attempts++;
            }
        }

        // Tenta manter o Mosquitto vivo se o WiFi estiver OK
        if (WiFi.status() == WL_CONNECTED && !client.connected()) {
            String clientId = "ESP32-Sensor-" + String(random(0xffff), HEX);
            if (!client.connect(clientId.c_str())) {
                vTaskDelay(pdMS_TO_TICKS(3000)); // Espera antes de tentar de novo
            }
        }
        
        // Se estiver tudo conectado, consome a fila e publica
        if (client.connected()) {
            // Processa todas as mensagens acumuladas na fila
            while (xQueueReceive(mqttQueue, &dataToSend, 0)) {
                snprintf(jsonBuffer, sizeof(jsonBuffer), "{\"leitura_adc\": %d, \"status\": \"%s\"}", 
                         dataToSend.leitura, dataToSend.status);
                client.publish("powerguard/sensores", jsonBuffer);
            }
            client.loop();
        } else {
            // Se estiver desconectado do MQTT, esvazia a fila para não acumular lixo antigo
            xQueueReceive(mqttQueue, &dataToSend, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

// ---------------- SETUP E LOOP ---------------------------------

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);

    client.setServer(mqtt_server, mqtt_port);

    sensorQueue = xQueueCreate(5, sizeof(SensorData));
    mqttQueue = xQueueCreate(5, sizeof(SensorData));

    xTaskCreatePinnedToCore(taskReadSensor, "ReadSensor", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskCalcularMedia, "CalcularMedia", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskPrintStatus, "PrintStatus", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(taskMQTT, "TaskMQTT", 4096, NULL, 1, NULL, 0);
}

void loop() {
    vTaskDelete(NULL); // Libera a tarefa loop (Economiza recursos já que usamos FreeRTOS)
}