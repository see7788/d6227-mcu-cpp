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
#include <serialTasknamespace.h>
#include <dz003Tasknamespace.h>
#include <filenamespace.h>
#define fileOs SPIFFS
#define uartDef Serial
#define EGBIG_CONFIG (2 << 0)
#define EGBIG_NET (1 << 0)
int taskindex = 0;
EventGroupHandle_t eg_Handle = xEventGroupCreate();
String GLOBALFILEPATH = "/config.json";
TaskHandle_t parsejsonStringSend_TaskHandle, parseJsonArraySend_TaskHandle, server_serial_TaskHandle, server_dz003_TaskHandle;
static StaticJsonDocument<3000> globalConfig;
void globalConfig_fromFile(void)
{
  if (!fileOs.begin(true))
  {
    ESP_LOGV("DEBUE", "!fileOs.begin(true)");
    return;
  }
  if (!fileOs.exists(GLOBALFILEPATH))
  {
    ESP_LOGV("DEBUE", " !fileOs.exists(GLOBALFILEPATH)");
    return;
  }
  File dataFile = fileOs.open(GLOBALFILEPATH);
  DeserializationError error = deserializeJson(globalConfig, dataFile);
  dataFile.close();
  if (error)
  {
    ESP_LOGV("DEBUE", "Error deserializing JSON:%s", error.c_str());
    return;
  }
  xEventGroupSetBits(eg_Handle, EGBIG_CONFIG);
  serializeJson(globalConfig, uartDef);
  //  serializeJsonPretty(globalConfig, uartDef);
}
void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
{
  // EventBits_t bits = xEventGroupWaitBits(eg_Handle, EGBIG_NET | EGBIG_NET | dz003_bit, pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupClearBits(eg_Handle, EGBIG_NET | BIT_2);
  char *er = (char *)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    ESP_LOGV("DEBUG", "ETH.localIP:%s", ETH.localIP().toString().c_str());
    xEventGroupSetBits(eg_Handle, EGBIG_NET);
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    ESP_LOGV("DEBUG", "WiFi.localIP:%s", WiFi.localIP().toString().c_str());
    xEventGroupSetBits(eg_Handle, EGBIG_NET);
    use = 1;
  }
  // char *data = eventData ? ((char *)eventData) : ((char *)"");
  // ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
  ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
}
void server_esp(void)
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
}
void sendEr(structTypenamespace::notifyString_t *strObj)
{
  if (strcmp(strObj->sendTo_name, "server_serial") == 0)
  {
    serialTasknamespace::send(strObj->msg);
  }
  else
  {
    strObj->msg = "sendTo_name undefind";
    serialTasknamespace::send(strObj->msg);
  }
}
void parseJsonArraySend(structTypenamespace::notifyJsonArray_t *arrObj)
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
    File file = fileOs.open(GLOBALFILEPATH, "w");
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
    msg = "[\"mcu pass\",\"" + api + "\"]";
  }
  structTypenamespace::notifyString_t strObj = {arrObj->sendTo_name, msg};
  sendEr(&strObj);
}
void parseStringSend(structTypenamespace::notifyString_t *strObj)
{
  DynamicJsonDocument doc(3000);
  DeserializationError error = deserializeJson(doc, strObj->msg);
  if (error)
  {
    strObj->msg = "[\"json pase error\",\"" + String(error.c_str()) + "\"]";
    sendEr(strObj);
  }
  else
  {
    JsonArray arr = doc.as<JsonArray>();
    structTypenamespace::notifyJsonArray_t arrObj = {strObj->sendTo_name, arr};
    parseJsonArraySend(&arrObj);
  }
}
void parseJsonArraySend_Task(void *nullparam)
{
  uint32_t ptr;
  structTypenamespace::notifyJsonArray_t *arrObj ;//= new structTypenamespace::notifyJsonArray_t();
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      arrObj = (structTypenamespace::notifyJsonArray_t *)ptr;
      parseJsonArraySend(arrObj);
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void parsejsonStringSend_Task(void *nullparam)
{
  uint32_t ptr;
  structTypenamespace::notifyString_t *strObj ;//= new structTypenamespace::notifyString_t();
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      strObj = (structTypenamespace::notifyString_t *)ptr;
      parseStringSend(strObj);
      ESP_LOGE("DEBUE", "%s", strObj->sendTo_name);
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void server_net(void)
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
}
void server_serial(void)
{
  Serial.onReceive([]()
                   {
     structTypenamespace::notifyString_t strObj = {
      globalConfig["server"]["serial"][0].as<const char *>(),
      Serial.readStringUntil('\n')
      };
  parseStringSend(&strObj); });
}
void server_dz003(void)
{
  JsonArray dz003 = globalConfig["server"]["dz003"].as<JsonArray>();
  dz003Tasknamespace::taskParam_t param = {dz003, parsejsonStringSend_TaskHandle};
  xTaskCreate(dz003Tasknamespace::mainTask, "server_dz003_Task", 1024 * 6, (void *)&param, taskindex++, &server_dz003_TaskHandle);
}
void setup(void)
{
  uartDef.begin(115200);
  server_esp();
  globalConfig_fromFile();
  xEventGroupWaitBits(eg_Handle, EGBIG_CONFIG, pdTRUE, pdTRUE, portMAX_DELAY);
  ESP_LOGE("DEBUE", "%s", "---------------EGBIG_CONFIG  SUCCESS----------------");
  xTaskCreate(parsejsonStringSend_Task, "parsejsonStringSend_Task", 1024 * 10, NULL, taskindex++, &parsejsonStringSend_TaskHandle);
  xTaskCreate(parseJsonArraySend_Task, "parseJsonArraySend_Task", 1024 * 10, NULL, taskindex++, &parseJsonArraySend_TaskHandle);
  server_serial();
  server_net();
  xEventGroupWaitBits(eg_Handle, EGBIG_NET, pdTRUE, pdTRUE, portMAX_DELAY);
  server_dz003();
  ESP_LOGE("DEBUE", "%s", "---------------EGBIG_NET SUCCESS------------------");
  // vTaskStartScheduler();
}
void loop(void)
{
  globalConfig.garbageCollect();
  vTaskDelay(10000);
}
