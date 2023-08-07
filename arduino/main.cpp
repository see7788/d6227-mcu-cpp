#include <Arduino.h>
#include <WiFi.h>
#include <stdio.h>
#include <String.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include <esp_mesh.h>
#include <esp_log.h>
#include <FS.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include "apiTasknamespace.h"
#include "serialTasknamespace.h"
#include "dz003Tasknamespace.h"
#include "fileOsFun.h"
#include "webServerFun.h"
#define fileOs SPIFFS
#define uartDef Serial
#define net_bit (1 << 0)
// #define net_bit (1 << 1)
EventGroupHandle_t eg_Handle = xEventGroupCreate();
String globalFilePath = "/config.json";
int taskindex = 20;
static StaticJsonDocument<3000> globalConfig;
void globalConfig_fromFile(void)
{
  if (!fileOs.begin(true))
  {
    ESP_LOGV("DEBUE", "!fileOs.begin(true)");
    return;
  }
  fileOsFun::listFilePrint(fileOs, "/", 0);
  if (!fileOs.exists(globalFilePath))
  {
    ESP_LOGV("DEBUE", " !fileOs.exists(globalFilePath)");
    return;
  }
  File dataFile = fileOs.open(globalFilePath);
  deserializeJson(globalConfig, dataFile);
  // serializeJson(globalConfig, uartDef);//
  serializeJsonPretty(globalConfig, uartDef);
  ESP_LOGV("DEBUG", "\n");
  dataFile.close();
  ESP_LOGV("DEBUG", "success");
}
TaskHandle_t apiTask_handle;
void apiTask(void *nullparam)
{
  uint32_t msgptr;
  static apiTasknamespace::NotifyObj_t obj;
  DynamicJsonDocument *doc;
  static String msg;
  static String api;
  static apiTasknamespace::send_t send;
  for (;;)
  {
    xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&msgptr, portMAX_DELAY);
    obj = *(apiTasknamespace::NotifyObj_t *)msgptr;
    doc = obj.doc;
    api = obj.doc[0].as<String>();
    if (api == "config_set")
    {
      for (auto kv : obj.doc[1].as<JsonObject>())
      {
        if (globalConfig.containsKey(kv.key()))
        {
          globalConfig[kv.key()].set(kv.value());
        };
      };
    }
    else if (api == "config_get")
    {
      doc[0] = "config_set";
      doc->add(globalConfig);
      msg = "";
      serializeJson(obj.doc, msg);
    }
    else if (api == "config_toFile")
    {
      File file = fileOs.open(globalFilePath, "w");
      serializeJson(globalConfig, file);
      file.close();
      msg = "[\"config_toFile\"]";
    }
    else if (api == "config_fromFile")
    {
      globalConfig_fromFile();
      doc[0] = "config_set";
      doc->add(globalConfig);
      msg = "";
      serializeJson(doc, msg);
    }
    else if (api == "restart")
    {
      msg = "[\"api restart\"]";
      ESP.restart(); // 重启复位esp32
    }
    else if (api == "state.egbits_get")
    {
      uint32_t ulBits = xEventGroupGetBits(eg_Handle); // 获取 Event Group 变量当前值
      msg = "[\"state.egbits_set\", [";
      for (int i = sizeof(ulBits) * 8 - 1; i >= 0; i--)
      { // 循环输出每个二进制位
        uint32_t mask = 1 << i;
        if (ulBits & mask)
        {
          msg += "1";
        }
        else
        {
          msg += "0";
        }
        if (i > 0)
        { // 添加逗号分隔符
          msg += ",";
        }
      }
      msg += "]]";
    }
    else if (api == "dz003.fa_set")
    {
      dz003Tasknamespace::fa_set(doc[1].as<bool>());
    }
    else if (api == "dz003.frequency_set")
    {
      dz003Tasknamespace::frequency_set(doc[1].as<bool>());
    }
    else if (api == "dz003.laba_set")
    {
      dz003Tasknamespace::laba_set(doc[1].as<bool>());
    }
    else if (api == "dz003.deng_set")
    {
      dz003Tasknamespace::deng_set(doc[1].as<bool>());
    }
    else
    {
      msg = "[\"api pass\"]";
    }
    send = apiTasknamespace::SendFunUndefined;
    if (obj.sendName == "serial")
    {
      send = serialTasknamespace::send;
    }
    send(msg);
  }
}

TaskHandle_t serialServer_sendTask_handle;
TaskHandle_t serialServer_onTask_handle;
void serialServerTask(void *nullparam)
{
  apiTasknamespace::strTaskParam_t sendTask_param;
  JsonArray serialconfig = globalConfig["server"]["serial"];
  sendTask_param.sendName = serialconfig[1].as<const char *>();
  xTaskCreate(apiTasknamespace::strTask, "serialServer_sendTask", 1024 * 4, (void *)&sendTask_param, taskindex--, &serialServer_sendTask_handle);
  serialTasknamespace::onTaskParam_t param;
  param.sendTask_handle = serialServer_sendTask_handle;
  param.baudRate = serialconfig[0].as<const int *>();
  xTaskCreate(serialTasknamespace::onTask, "serialServerTask", 1024 * 4, (void *)&param, taskindex--, &serialServer_onTask_handle);
  vTaskDelete(NULL);
}
TaskHandle_t dz003_sendTask_handle;
dz003Tasknamespace::taskParam_t dz003_param;
TaskHandle_t dz003_handle;
void dz003Task(void *nullparam)
{
  JsonObject c = globalConfig["dz003"].as<JsonObject>();
  String name = c["init"].as<String>();
  if (c.containsKey("taskA") && name == "taskA")
  {
    dz003_param.config = c["taskA"].as<JsonArray>();
    if (pdPASS != xTaskCreate(dz003Tasknamespace::taskA, "dz003::taskA", 1024 * 6, (void *)&dz003_param, taskindex++, NULL))
    {
      ESP_LOGE("debug", "dz003::taskA  error");
    }
  }
  else if (c.containsKey("taskB") && name == "taskB")
  {
    dz003_param.config = c["taskB"].as<JsonArray>();
    if (pdPASS != xTaskCreate(dz003Tasknamespace::taskB, "dz003::taskB", 1024 * 6, (void *)&dz003_param, taskindex++, NULL))
    {
      ESP_LOGE("debug", "dz003::taskB  error");
    }
  }
  else
  {
    ESP_LOGE("debug", "dz003::false ");
  }
  vTaskDelete(NULL);
}
// webServerFun::taskPtr_t webServerTaskPtr;
void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
{
  // EventBits_t bits = xEventGroupWaitBits(eg_Handle, net_bit | net_bit | dz003_bit, pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupSetBits(eg_Handle, net_bit);
  // xEventGroupClearBits(myEventGroup, net_bit | BIT_2);
  char *er = (char *)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    ESP_LOGV("DEBUG", "ETH.localIP:%s", ETH.localIP().toString().c_str());
    xEventGroupSetBits(eg_Handle, net_bit);
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    ESP_LOGV("DEBUG", "WiFi.localIP:%s", WiFi.localIP().toString().c_str());
    xEventGroupSetBits(eg_Handle, net_bit);
    use = 1;
  }
  // char *data = eventData ? ((char *)eventData) : ((char *)"");
  // ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
  ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
}

// void webServerCreate(void)
// {
//   sendEr_set("webServer");
//   webServerTaskPtr.taskindex = &taskindex;
//   webServerTaskPtr.config = globalConfig["webServer"].as<JsonObject>();
//   if (pdPASS != xTaskCreate(webServerFun::taskA, "webServerTask", 1024 * 6, (void *)&webServerTaskPtr, taskindex++, NULL))
//   {
//     ESP_LOGE("debug", "webServerTask  error");
//   }
// }
void netServerCreate(void)
{
  // init:ap|sta|eth|ap+eth|ap+sta|
  JsonObject c = globalConfig["netServer"].as<JsonObject>();
  String init = c["init"].as<String>();
  ESP_LOGD("start", "%s", init.c_str());
  if (init == "ap")
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(c["ap"]["ssid"].as<const char *>());
  }
  else if (init == "sta")
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(c["sta"]["ssid"].as<const char *>(), c["sta"]["password"].as<const char *>());
  }
  else if (init == "eth")
  {
    dz003Tasknamespace::eth_begin();
  }
  else if (init == "ap+sta")
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(c["ap"]["ssid"].as<const char *>());
    WiFi.begin(c["sta"]["ssid"].as<const char *>(), c["sta"]["password"].as<const char *>());
  }
  else if (init == "ap+eth")
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(c["ap"]["ssid"].as<const char *>());
    dz003Tasknamespace::eth_begin();
  }
  else
  {
    ESP_LOGE("debug", "init false");
  }
}

void setup(void)
{
  uartDef.begin(115200);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap(), ESP_OK);
  ESP_ERROR_CHECK(esp_task_wdt_init(20000, false)); // 初始化看门狗
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  ESP_ERROR_CHECK(esp_task_wdt_status(NULL));
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  globalConfig_fromFile();
  xTaskCreate(apiTask, "apiTask", 1024 * 6, NULL, taskindex--, &apiTask_handle);
  xTaskCreate(serialServerTask, "serialServerTask", 1024 * 2, NULL, taskindex--, NULL);
  xTaskCreate(dz003Task, "dz003Task", 1024 * 2, NULL, taskindex--, NULL);
  netServerCreate();
  // webServerCreate();
  // serialOnServerCreate();
  vTaskStartScheduler();
}
void loop(void)
{
  globalConfig.garbageCollect();
  vTaskDelay(10000);
}
