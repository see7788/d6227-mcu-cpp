#ifndef apiTasknamespace_h
#define apiTasknamespace_h
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ArduinoJson.h>

namespace apiTasknamespace
{

    typedef void (*send_t)(String);
    void SendFunUndefined(String message)
    {
        ESP_LOGE("SendNull", "%s", message.c_str());
    }
    typedef struct
    {
        const char *sendName;
        DynamicJsonDocument *doc;
    } NotifyObj_t;
    typedef struct
    {
        const char *sendName;
        TaskHandle_t sendTask_handle;
    } strTaskParam_t;
    // 这是主任务的例子
    // void apiTask_demo(void *nullparam)
    // {
    //     static apiTasknamespace::NotifyObj_t obj;
    //     static String msg;
    //     static String api;
    //     static apiTasknamespace::send_t send;
    //     for (;;)
    //     {
    //         xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&msgptr, portMAX_DELAY);
    //         obj = *(NotifyObj_t *)msgptr;
    //     }
    // }
    void strTask(void *taskParam)
    {
        strTaskParam_t *param = (strTaskParam_t *)taskParam;
        uint32_t msgptr;
        static String msg;
        static DynamicJsonDocument doc(3000);
        for (;;)
        {
            xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&msgptr, portMAX_DELAY);
            msg = (char *)msgptr;
            doc.clear();
            DeserializationError error = deserializeJson(doc, msg);
            if (error)
            {
                doc.clear();
                doc[0] = "Json error" + String(error.c_str());
                doc[1] = msg;
            }
            NotifyObj_t obj = {param->sendName, &doc};
            xTaskNotify(param->sendTask_handle, (uint32_t)&obj, eSetValueWithOverwrite);
        }
    }
    // TaskHandle_t sendTask_Handle;
    //  xTaskCreate(TaskFun, "onSerialErTask", 1024 * 6, (void *)taskFun, *(op.taskindex)++, &task_Handle))
    //  xTaskNotify(param->sendTask_Handle, (uint32_t)&serialOn_db, eSetValueWithOverwrite);
}

#endif