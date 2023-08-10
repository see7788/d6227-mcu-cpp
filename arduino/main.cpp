#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include <esp_mesh.h>
#include <esp_log.h>
#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <structTypenamespace.h>
#include "serialTasknamespace.h"
#include "dz003Tasknamespace.h"
#include "filenamespace.h"
#define fileOs SPIFFS
#define uartDef Serial
#define net_bit (1 << 0)
int taskindex = 0;
EventGroupHandle_t eg_Handle = xEventGroupCreate();
String globalFilePath = "/config.json";
TaskHandle_t resJsonArray_TaskHandle, resStr_TaskHandle, server_serial_TaskHandle, server_net_TaskHandle, server_dz003_TaskHandle;
static StaticJsonDocument<3000> globalConfig;
void globalConfig_fromFile(void)
{
  ESP_LOGV("end", "start");
  if (!fileOs.begin(true))
  {
    ESP_LOGV("DEBUE", "!fileOs.begin(true)");
    return;
  }
  if (!fileOs.exists(globalFilePath))
  {
    ESP_LOGV("DEBUE", " !fileOs.exists(globalFilePath)");
    return;
  }
  File dataFile = fileOs.open(globalFilePath);
  deserializeJson(globalConfig, dataFile);
  // serializeJson(globalConfig, uartDef);//
  serializeJsonPretty(globalConfig, uartDef);
  dataFile.close();
  ESP_LOGV("end", "success");
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
void espTask(void *nullparam)
{
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
  vTaskDelete(NULL);
}
void sendEr(structTypenamespace::notifyString_t *strObj)
{
  if (strcmp(strObj->sendTo_name, "server_serial") == 0)
  {
    serialTasknamespace::send(strObj->msg);
  }
  else
  {
    ESP_LOGV("SendNull", "%s", strObj->msg.c_str());
  }
}
void apiSendEr(structTypenamespace::notifyJsonArray_t *arrObj)
{
  JsonArray doc = arrObj->msg;
  String api = doc[0].as<String>();
  String msg;
  if (api == "config_set")
  {
    for (auto kv : doc[1].as<JsonObject>())
    {
      if (globalConfig.containsKey(kv.key()))
      {
        globalConfig[kv.key()].set(kv.value());
      };
    };
  }
  else if (api == "config_get")
  {
    doc[0].set("config_set");
    doc[1].set(globalConfig);
    serializeJson(doc, msg);
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
    doc[0].set("config_set");
    doc[1].set(globalConfig);
    serializeJson(doc, msg);
  }
  else if (api == "restart")
  {
    msg = "[\"api restart\"]";
    ESP.restart(); // 重启复位esp32
  }
  else if (api == "state_get")
  {
    uint32_t ulBits = xEventGroupGetBits(eg_Handle); // 获取 Event Group 变量当前值
    doc[0].set("state_set");
    DynamicJsonDocument info(100);
    JsonObject egBit = info.createNestedObject("egBit");
    for (int i = sizeof(ulBits) * 8 - 1; i >= 0; i--)
    { // 循环输出每个二进制位
      uint32_t mask = 1 << i;
      if (ulBits & mask)
      {
        egBit[String(i)] = true;
      }
      else
      {
        egBit[String(i)] = false;
      }
    }
    egBit["localIP"] = "";
    doc[1].set(egBit);
    serializeJson(doc, msg);
  }
  else if (api == "dz003State")
  {
    msg = dz003Tasknamespace::state();
  }
  else if (api == "dz003.fa_set")
  {
    dz003Tasknamespace::fa_set(doc[1].as<bool>());
    msg = dz003Tasknamespace::state();
  }
  else if (api == "dz003.frequency_set")
  {
    dz003Tasknamespace::frequency_set(doc[1].as<bool>());
    msg = dz003Tasknamespace::state();
  }
  else if (api == "dz003.laba_set")
  {
    dz003Tasknamespace::laba_set(doc[1].as<bool>());
    msg = dz003Tasknamespace::state();
  }
  else if (api == "dz003.deng_set")
  {
    dz003Tasknamespace::deng_set(doc[1].as<bool>());
    msg = dz003Tasknamespace::state();
  }
  else
  {
    msg = "[\"mcu pass api:" + api + "\"]";
  }
  structTypenamespace::notifyString_t strObj = {arrObj->sendTo_name, msg};
  sendEr(&strObj);
}
void resJsonArray_Task(void *nullparam)
{
  uint32_t ptr;
  structTypenamespace::notifyJsonArray_t *arrObj;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      arrObj = (structTypenamespace::notifyJsonArray_t *)ptr;
      apiSendEr(arrObj);
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void resString_Task(void *nullparam)
{
  uint32_t ptr;
  structTypenamespace::notifyString_t *strObj;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      strObj = (structTypenamespace::notifyString_t *)ptr;
      DynamicJsonDocument doc(3000);
      DeserializationError error = deserializeJson(doc, strObj->msg);
      if (error)
      {
        sendEr(strObj);
        ESP_LOGV("resString_Task", "err");
      }
      else
      {
        ESP_LOGV("resString_Task", "success");
        JsonArray arr = doc.as<JsonArray>();
        structTypenamespace::notifyJsonArray_t arrObj = {strObj->sendTo_name, arr};
        apiSendEr(&arrObj);
      }
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void server_net_Task(void *nullparam)
{
  // init:ap|sta|eth|ap+eth|ap+sta|
  JsonObject c = globalConfig["server"]["net"].as<JsonObject>();
  String init = c["init"].as<String>();
  ESP_LOGD("start", "%s", init.c_str());
  if (init == "ap")
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(c["ap"][1].as<const char *>());
  }
  else if (init == "sta")
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(c["sta"][0].as<const char *>(), c["sta"][1].as<const char *>());
  }
  else if (init == "eth")
  {
    dz003Tasknamespace::eth_begin();
  }
  else if (init == "ap+sta")
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(c["ap"][1].as<const char *>());
    WiFi.begin(c["sta"][0].as<const char *>(), c["sta"][1].as<const char *>());
  }
  else if (init == "ap+eth")
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(c["ap"][0].as<const char *>());
    dz003Tasknamespace::eth_begin();
  }
  else
  {
    ESP_LOGE("debug", "init false");
  }
  ESP_LOGD("end", "%s", init.c_str());
  vTaskDelete(NULL);
}
void server_dz003_Task()
{
  void (*taskFunc)(void *) = nullptr;
  JsonObject config = globalConfig["server"]["dz003"].as<JsonObject>();
  const char *init = config["init"].as<const char *>();
  // ESP_LOGV("debug", "init=%s,sendto=%s", init, config["sendTo_name"].as<const char *>());
  if (strcmp(init, "taskA") == 0)
  {
    taskFunc = dz003Tasknamespace::taskA;
  }
  else if (strcmp(init, "taskB") == 0)
  {
    taskFunc = dz003Tasknamespace::taskB;
  }
  else
  {
    ESP_LOGE("debug", "dz003::false ");
  }
  if (taskFunc != nullptr)
  {
    dz003Tasknamespace::taskParam_t param = {config[init].as<JsonArray>(), config["sendTo_name"].as<const char *>(), resStr_TaskHandle};
    xTaskCreate(dz003Tasknamespace::taskB, "server_dz003_Task", 1024 * 6, (void *)&param, taskindex++, &server_dz003_TaskHandle);
  }
}
void setup(void)
{
  uartDef.begin(115200);
  xTaskCreate(espTask, "espTask", 1024 * 2, NULL, taskindex++, NULL);
  globalConfig_fromFile();
  xTaskCreate(resString_Task, "resString_Task", 1024 * 10, NULL, taskindex++, &resStr_TaskHandle);
  serialTasknamespace::onTaskParam_t server_serial_Param = {globalConfig["server"]["serial"][0].as<const char *>(), resStr_TaskHandle};
  xTaskCreate(serialTasknamespace::onTask, "server_serial_Task", 1024 * 4, (void *)&server_serial_Param, taskindex++, &server_serial_TaskHandle);
  xTaskCreate(server_net_Task, "server_net_Task", 1024 * 6, NULL, taskindex++, &server_net_TaskHandle);
  server_dz003_Task();
  // vTaskStartScheduler();
}
void loop(void)
{
  globalConfig.garbageCollect();
  vTaskDelay(10000);
}
