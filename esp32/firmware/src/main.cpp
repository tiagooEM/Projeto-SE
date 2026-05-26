#include <Arduino.h>
#include <math.h>

#define SCT_PIN 34

// ADC ESP32
#define ADC_RESOLUTION 4095.0
#define ADC_VREF 3.3

// Offset criado pelo divisor
#define ADC_OFFSET 1.65

// Quantidade de amostras
#define NUM_SAMPLES 1000

// Sensibilidade aproximada SCT-013 20A/1V
#define SCT_SCALE 20.0

// Limite para detectar energia
#define CURRENT_THRESHOLD 0.15

QueueHandle_t currentQueue;

typedef struct {
    float current;
} CurrentData;


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

void taskReadCurrent(void *pvParameters) {

    CurrentData data;

    while (true) {

        data.current = readCurrentRMS();

        xQueueSend(currentQueue, &data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void taskDetectPower(void *pvParameters) {

    CurrentData received;

    while (true) {

        if (xQueueReceive(currentQueue, &received, portMAX_DELAY)) {

            Serial.print("Corrente RMS: ");
            Serial.print(received.current);
            Serial.println(" A");

            // DETECÇÃO
            if (received.current < CURRENT_THRESHOLD) {

                Serial.println("⚠ POSSIVEL CORTE DE ENERGIA!");

            } else {

                Serial.println("✅ Energia OK");
            }

            Serial.println("-------------------------");
        }
    }
}

void setup() {

    Serial.begin(115200);

    analogReadResolution(12);

    analogSetAttenuation(ADC_11db);

    currentQueue = xQueueCreate(5, sizeof(CurrentData));

    xTaskCreatePinnedToCore(
        taskReadCurrent,
        "ReadCurrent",
        4096,
        NULL,
        1,
        NULL,
        0
    );

    xTaskCreatePinnedToCore(
        taskDetectPower,
        "DetectPower",
        4096,
        NULL,
        1,
        NULL,
        1
    );
}

void loop() {
}