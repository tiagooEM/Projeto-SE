#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// estrutura compartilhada
typedef struct {
    float x;
    float y;
    float z;
} AccelData;

QueueHandle_t accelQueue;

// ---------- TASK 1 ----------
void taskReadSensor(void *pvParameters) {

    sensors_event_t a, g, temp;
    AccelData data;

    while(true) {

        mpu.getEvent(&a, &g, &temp);

        data.x = a.acceleration.x;
        data.y = a.acceleration.y;
        data.z = a.acceleration.z;

        xQueueSend(accelQueue, &data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ---------- TASK 2 ----------
void taskProcessData(void *pvParameters) {

    AccelData received;

    while(true) {

        if(xQueueReceive(accelQueue, &received, portMAX_DELAY)) {

            Serial.print("X: ");
            Serial.print(received.x);
            Serial.print(" Y: ");
            Serial.print(received.y);
            Serial.print(" Z: ");
            Serial.println(received.z);
        }
    }
}

void setup() {

    Serial.begin(115200);
    Wire.begin(21,22);

    if(!mpu.begin()) {
        Serial.println("MPU6050 nao encontrado");
        while(true);
    }

    accelQueue = xQueueCreate(10, sizeof(AccelData));

    // Task leitura - Core 0
    xTaskCreatePinnedToCore(
        taskReadSensor,
        "ReadSensor",
        4096,
        NULL,
        1,
        NULL,
        0);

    // Task processamento - Core 1
    xTaskCreatePinnedToCore(
        taskProcessData,
        "ProcessData",
        4096,
        NULL,
        1,
        NULL,
        1);
}

void loop() {
    // vazio (FreeRTOS controla tudo)
}