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
#include "dz003.h"
#include "fileOsFun.h"
#include "serialFun.h"
#include "webServerFun.h"
#define fileOs SPIFFS
#define uartDef Serial
#define net_bit (1 << 0)
// #define net_bit (1 << 1)
EventGroupHandle_t eg_Handle = xEventGroupCreate();
String globalFilePath = "/config.json";
int taskindex;
static StaticJsonDocument<3000> globalConfig;
dz003::taskPtr_t dz003taskPtr;
serialFun::onTaskPtr_t serialServerTaskPtr;
webServerFun::taskPtr_t webServerTaskPtr;
typedef void (*send_t)(String);
void sendEr_set(String jsonkey)
{
  String name = globalConfig[jsonkey]["sendFun"].as<String>();
  send_t fun;
  if (name == "webServer")
  {
    fun = webServerFun::send;
  }
  else if (name == "serial")
  {
    fun = serialFun::send;
  }
  else
  {
    fun = serialFun::send;
  }
  if (globalConfig.containsKey(jsonkey))
  {
    if (jsonkey == "dz003")
    {
      dz003taskPtr.sendFun = fun;
    }
    else if (jsonkey == "webServer")
    {
      webServerTaskPtr.sendFun = fun;
    }
    else if (jsonkey == "serialOnServer")
    {
      serialServerTaskPtr.sendFun = fun;
    }
    else
    {
      ESP_LOGE("error", "jsonkey error");
    }
  }
}
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
void sendErTask(void *sendptr)
{
  ESP_LOGV("start", "");
  send_t send = (send_t)sendptr;
  uint32_t msgptr;
  String msg;
  for (;;)
  {
    xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&msgptr, portMAX_DELAY);
    msg = *(String *)msgptr;
    // ESP_LOGV("--------222-------", "%s/n",msg.c_str());
    StaticJsonDocument<3000> docdb;
    DeserializationError error = deserializeJson(docdb, msg);
    // ESP_LOGD("debug", "%s", msg.c_str());
    if (error)
    {
      msg = "[\"Json error" + msg + "\"]";
      send(msg);
    }
    else
    {
      JsonArray doc = docdb.as<JsonArray>();
      String api = doc[0].as<String>();
      if (api == "config_set")
      {
        for (auto kv : doc[1].as<JsonObject>())
        {
          if (globalConfig.containsKey(kv.key()))
          {
            globalConfig[kv.key()].set(kv.value());
            sendEr_set(String(kv.key().c_str()));
          };
        };
        send(msg);
      }
      else if (api == "globalConfig_set")
      {
        for (auto kv : doc[1].as<JsonObject>())
        {
          if (globalConfig.containsKey(kv.key()))
          {
            globalConfig[kv.key()].set(kv.value());
          };
        };
        send(msg);
      }
      else if (api == "globalConfig_get")
      {
        doc[0] = "globalConfig_set";
        doc.add(globalConfig);
        msg = "";
        serializeJson(doc, msg);
        send(msg);
      }
      else if (api == "globalConfig_toFile")
      {
        File file = fileOs.open(globalFilePath, "w");
        serializeJson(globalConfig, file);
        file.close();
        msg = "[\"globalConfig_toFile\"]";
        send(msg);
      }
      else if (api == "globalConfig_fromFile")
      {
        globalConfig_fromFile();
        doc[0] = "globalConfig_set";
        doc.add(globalConfig);
        msg = "";
        serializeJson(doc, msg);
        send(msg);
      }
      else if (api == "espRestart")
      {
        send("[\"api espRestart\"]");
        send(msg);
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
        send(msg);
      }
      else if (api == "dz003.fa_set")
      {
        dz003::fa_set(doc[1].as<bool>());
        dz003::stateSend(send);
      }
      else if (api == "dz003.frequency_set")
      {
        dz003::frequency_set(doc[1].as<bool>());
        dz003::stateSend(send);
      }
      else if (api == "dz003.laba_set")
      {
        dz003::laba_set(doc[1].as<bool>());
        dz003::stateSend(send);
      }
      else if (api == "dz003.deng_set")
      {
        dz003::deng_set(doc[1].as<bool>());
        dz003::stateSend(send);
      }
      else
      {
        msg = "[\"api pass\"]";
        send(msg);
      }
    }
  }
}
void webServerCreate(void)
{
  sendEr_set("webServer");
  webServerTaskPtr.taskindex = &taskindex;
  webServerTaskPtr.sendErTaskFun = sendErTask;
  webServerTaskPtr.config = globalConfig["webServer"].as<JsonObject>();
  if (pdPASS != xTaskCreate(webServerFun::taskA, "webServerTask", 1024 * 6, (void *)&webServerTaskPtr, taskindex++, NULL))
  {
    ESP_LOGE("debug", "webServerTask  error");
  }
}
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
    dz003::eth_begin();
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
    dz003::eth_begin();
  }
  else
  {
    ESP_LOGE("debug", "init false");
  }
}
void serialOnServerCreate(void)
{
  sendEr_set("serialOnServer");
  serialServerTaskPtr.taskindex = &taskindex;
  serialServerTaskPtr.sendErTaskFun = sendErTask;
  serialFun::onTaskACreate(&serialServerTaskPtr);
}
void dz003TaskCreate(void)
{
  JsonObject c = globalConfig["dz003"].as<JsonObject>();
  sendEr_set("dz003");
  String name = c["init"].as<String>();
  if (c.containsKey("taskA") && name == "taskA")
  {
    dz003taskPtr.config = c["taskA"].as<JsonArray>();
    if (pdPASS != xTaskCreate(dz003::taskA, "dz003::taskA", 1024 * 6, (void *)&dz003taskPtr, taskindex++, NULL))
    {
      ESP_LOGE("debug", "dz003::taskA  error");
    }
  }
  else if (c.containsKey("taskB") && name == "taskB")
  {
    dz003taskPtr.config = c["taskB"].as<JsonArray>();
    if (pdPASS != xTaskCreate(dz003::taskB, "dz003::taskB", 1024 * 6, (void *)&dz003taskPtr, taskindex++, NULL))
    {
      ESP_LOGE("debug", "dz003::taskB  error");
    }
  }
  else
  {
    ESP_LOGE("debug", "dz003::false ");
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
  netServerCreate();
  webServerCreate();
  serialOnServerCreate();
  dz003TaskCreate();
}
void loop(void)
{
  globalConfig.garbageCollect();
  vTaskDelay(10000);
}
