#ifndef serialTasknamespace_h
#define serialTasknamespace_h
#include <iostream>
#include <Arduino.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
namespace serialTasknamespace
{
    typedef struct
    {
        const int *baudRate;
        TaskHandle_t sendTask_handle;
    } onTaskParam_t;
    void onTask(void *funparam)
    {
        onTaskParam_t *param = (onTaskParam_t *)funparam;
        static String db;
        ESP_LOGV("start", "");
        for (;;)
        {
            if (Serial.available())
            {
                db = Serial.readStringUntil('\n');
                xTaskNotify(param->sendTask_handle, (uint32_t)&db, eSetValueWithOverwrite);
            }
            vTaskDelay(1000);
        }
    }
    void send(String str)
    {
        Serial.println(str);
    }
}

#endif