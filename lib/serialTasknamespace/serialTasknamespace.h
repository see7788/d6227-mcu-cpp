#ifndef serialTasknamespace_h
#define serialTasknamespace_h
#include <iostream>
#include <Arduino.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <structTypenamespace.h>
#include <HardwareSerial.h>
namespace serialTasknamespace
{
    void send(String str)
    {
        Serial.println(str);
    }
    // Serial.onReceive([]()
    //                  {
    //  structTypenamespace::notifyString_t strObj = {
    //   globalConfig["server"]["serial"][0].as<const char *>(),
    //   Serial.readStringUntil('\n')
    //   };
    //    parseSend(&strObj); });

    // typedef struct
    // {
    //     // int *baudRate;
    //     // int *rxGpio;
    //     const char *sendTo_name;
    //     TaskHandle_t sendTo_taskHandle;
    // } onTaskParam_t;

    // SemaphoreHandle_t SerialSemaphore_Handle;
    // QueueHandle_t SerialQueue_Handle;
    // void IRAM_ATTR onTask_serialEvent()
    // {
    //     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    //     String *data = new String(Serial.readStringUntil('\n'));
    //     xQueueSendFromISR(SerialQueue_Handle, &data, &xHigherPriorityTaskWoken);
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }

    // void onTask(void *funparam)
    // {
    //     onTaskParam_t param = *(onTaskParam_t *)funparam;
    //     structTypenamespace::notifyString_t obj = {param.sendTo_name};
    //     SerialQueue_Handle = xQueueCreate(3000, sizeof(String *));
    //     attachInterrupt(digitalPinToInterrupt(3), onTask_serialEvent, FALLING);
    //     while (true)
    //     {
    //         String *receivedData;
    //         if (xQueueReceive(SerialQueue_Handle, &receivedData, portMAX_DELAY) == pdTRUE)
    //         {
    //             obj.msg = *receivedData;
    //             xTaskNotify(param.sendTo_taskHandle, (uint32_t)&obj, eSetValueWithOverwrite);
    //             delete receivedData;
    //         }
    //     }
    // }
    // serialTasknamespace::onTaskParam_t server_serial_Param = {globalConfig["server"]["serial"][0].as<const char *>(), resStr_TaskHandle};
    // xTaskCreate(serialTasknamespace::onTask, "server_serial_Task", 1024 * 4, (void *)&server_serial_Param, taskindex++, &server_serial_TaskHandle);
}

#endif