#ifndef serialFun_h
#define serialFun_h
#include <Arduino.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
namespace serialFun
{
    TaskHandle_t onTask_Handle;
    String serialOn_db;
    typedef struct
    {
        int *taskindex;
        void (*onErTaskFun)(void *);
        void (*sendFun)(String);
    } onTaskPtr_t;
    void onTaskA(void *ptr)
    {
        onTaskPtr_t op = *(onTaskPtr_t *)ptr;
        if (pdPASS == xTaskCreate(op.onErTaskFun, "onSerialErTask", 1024 * 6, (void *)op.sendFun, *(op.taskindex)++, &onTask_Handle))
        {
            ESP_LOGE("debug", "onSerialErTask success");
        }
        else
        {
            ESP_LOGE("debug", "onSerialErTask error");
        }
        ESP_LOGV("start", "");
        for (;;)
        {
            if (Serial.available())
            {
                serialOn_db = Serial.readStringUntil('\n');
                xTaskNotify(onTask_Handle, (uint32_t)&serialOn_db, eSetValueWithOverwrite);
            }
            vTaskDelay(1000);
        }
    }
    void onTaskACreate(onTaskPtr_t *op)
    {
        if (pdPASS != xTaskCreate(onTaskA, "serialOnServerTask", 1024 * 4, (void *)op, *(op->taskindex)++, NULL))
        {
            ESP_LOGE("debug", "serialOnServerTask  error");
        }
    }
    void send(String str)
    {
        Serial.println(str);
    }
    // void serialEvent(void)
    // {
    //   if (uartDef.available())
    //   {
    //     serialOn_db = uartDef.readStringUntil('\n');
    //     xTaskNotify(onSerialErTask_Handle, (uint32_t)&serialOn_db, eSetValueWithOverwrite);
    //   }
    // }
    // void serialOnServerCreate(void)
    // {
    //   xTaskCreate(onErTask, "onSerialErTask", 1024 * 6, (void *)serialServer_send, taskindex++, &onSerialErTask_Handle);
    //   attachInterrupt(digitalPinToInterrupt(3), serialEvent, FALLING);
    // }
}

#endif