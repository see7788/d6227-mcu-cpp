#include <String>
#include <tuple>
#include <ESP.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include <esp_mesh.h>
#include <esp_log.h>
#include <FS.h>
#include <WiFi.h>
#include <Arduino.h>
#include <IPAddress.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <structTypenamespace.h>
#include <MySerial.h>
#include <MyWebServer.h>
#include <dz003namespace.h>
#include <MyFs.h>
#include <MyNet.h>
#include <a7129namespace.h>
#define EGBIG_SERVER_CONFIGFS (1 << 0)
#define EGBIG_SERVER_NET (2 << 0)
using namespace std;
struct state_t
{
  SemaphoreHandle_t configLock;
  MyFs *server_configFs;
  MyNet *server_net;
  MySerial *server_serial;
  MyWebServer *server_webserver;
  dz003namespace::taskParam_t *server_dz003_taskParam;
  a7129namespace::taskParam_t *server_ybl_taskParam;
  String locIp;
  int taskindex;
} state;
struct config_t
{
  tuple<String, String, String, String> server_env;
  tuple<String> client_html;
  MySerial::config_t server_serial;
  MyNet::config_t server_net;
  MyWebServer::config_MyWebServer_t server_html;
  dz003namespace::config_t server_dz003;
  a7129namespace::config_t server_ybl;
} config;
EventGroupHandle_t eg_Handle;
TaskHandle_t stdStringTaskHandle, jsonArrayTaskHandle, server_serialTaskHandle, server_dz003TaskHandle;
void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
{
  // EventBits_t bits = xEventGroupWaitBits(eg_Handle, EGBIG_SERVER_NET | EGBIG_SERVER_NET , pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupClearBits(eg_Handle, EGBIG_SERVER_NET | BIT_2);
  char *er = (char *)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    state.locIp = ETH.localIP().toString().c_str();
    xEventGroupSetBits(eg_Handle, EGBIG_SERVER_NET);
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    state.locIp = WiFi.localIP().toString().c_str();
    xEventGroupSetBits(eg_Handle, EGBIG_SERVER_NET);
    use = 1;
  }
  // char *data = eventData ? ((char *)eventData) : ((char *)"");
  // ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
  ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
}
void config_set(JsonObject &obj)
{
  // JsonArray arr;
  // if (obj.containsKey("server_configFs"))
  // {
  //   arr = obj["server_configFs"].as<JsonArray>();
  //   config.server_configFs = make_tuple(
  //       arr[0].as<String>());
  // }
  // if (obj.containsKey("server_serial"))
  // {
  //   arr = obj["server_serial"].as<JsonArray>();
  //   config.server_serial = make_tuple(
  //       arr[0].as<int>(),
  //       arr[1].as<int8_t>(),
  //       arr[2].as<int8_t>(),
  //       arr[4].as<String>());
  // }
  // if (obj.containsKey("server_net"))
  // {
  //   arr = obj["server_net"].as<JsonArray>();
  //   // config.server_net = make_tuple(
  //   //     arr[0].as<int>(),
  //   //     arr[1].as<int8_t>(),
  //   //     arr[2].as<int8_t>(),
  //   //     arr[4].as<String>());
  // }
  // if (obj.containsKey("server_dz003"))
  // {
  //   arr = obj["server_dz003"].as<JsonArray>();
  //   config.server_dz003 = make_tuple(
  //       arr[0].as<int>(),
  //       arr[1].as<int>(),
  //       arr[2].as<int>(),
  //       arr[3].as<int>(),
  //       arr[4].as<String>());
  // }
}
void config_get(JsonObject &obj)
{
  // JsonObject env = obj.createNestedObject("env");
  // JsonArray server_configFs = obj.createNestedArray("server_configFs");
}
void sendEr(structTypenamespace::notifyString_t &strObj)
{
  if (strObj.sendTo_name == "server_serial")
  {
    state.server_serial->println(strObj.msg.c_str());
  }
  else
  {
    strObj.msg = "[\"sendTo_name undefind\"]";
    state.server_serial->println(strObj.msg.c_str());
  }
}
void jsonArrayParse(structTypenamespace::notifyJsonArray_t &arrObj)
{
  if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
  {
    JsonArray arr = arrObj.msg;
    String api = arr[0].as<String>();
    if (api == "config_set")
    {
      JsonObject obj = arr[1].as<JsonObject>();
      config_set(obj);
    }
    else if (api == "config_get")
    {
      arr[0].set("config_set");
      JsonObject obj = arr.createNestedObject();
      config_get(obj);
    }
    else if (api == "config_toFile")
    {
      JsonObject obj = arr.createNestedObject();
      config_get(obj);
      int success = state.server_configFs->writeFile(obj);
      if (success < 1)
      {
        arr[0].set("config_toFile error");
      }
    }
    else if (api == "config_fromFile")
    {
      DynamicJsonDocument doc(3000);
      DeserializationError error = state.server_configFs->readFile(doc);
      if (error)
      {
        arr[0].set("config_fromFile error");
        arr[1].set(error.c_str());
      }
      else
      {
        JsonObject c = doc.as<JsonObject>();
        JsonObject obj = arr.createNestedObject();
        obj.set(c);
        config_set(obj);
        arr[0].set("config_set");
      }
    }
    else if (api == "restart")
    {
      ESP.restart();
    }
    else if (api == "state_get")
    {
      uint32_t ulBits = xEventGroupGetBits(eg_Handle); // 获取 Event Group 变量当前值
      arr.clear();
      arr[0].set("state_set");
      JsonObject obj = arr.createNestedObject();
      JsonObject egBit = obj.createNestedObject("egBit");
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
      obj["locIp"] = state.locIp;
    }
    else
    {
      auto getdz003State = [&arr]()
      {
        arr[0].set("dz003State");
        JsonObject obj = arr[1].as<JsonObject>();
        dz003namespace::state(obj);
      };
      if (api == "dz003.state")
      {
        getdz003State();
      }
      else if (api == "dz003.fa_set")
      {
        dz003namespace::fa_set(arr[1].as<bool>());
        getdz003State();
      }
      else if (api == "dz003.frequency_set")
      {
        dz003namespace::frequency_set(arr[1].as<bool>());
        getdz003State();
      }
      else if (api == "dz003.laba_set")
      {
        dz003namespace::laba_set(arr[1].as<bool>());
        getdz003State();
      }
      else if (api == "dz003.deng_set")
      {
        dz003namespace::deng_set(arr[1].as<bool>());
        getdz003State();
      }
      else
      {
        arr[0].set("mcu pass");
        arr[1].set(api);
      }
    }
    String msg;
    serializeJson(arr, msg);
    structTypenamespace::notifyString_t strObj = {arrObj.sendTo_name, msg};
    sendEr(strObj);
    xSemaphoreGive(state.configLock);
  }
}
void stdStringParse(structTypenamespace::notifyString_t &strObj)
{
  DynamicJsonDocument doc(3000);
  DeserializationError error = deserializeJson(doc, strObj.msg);
  if (error)
  {
    strObj.msg = "[\"json pase error\",\"" + String(error.c_str()) + "\"]";
    sendEr(strObj);
  }
  else
  {
    JsonArray arr = doc.as<JsonArray>();
    structTypenamespace::notifyJsonArray_t arrObj = {strObj.sendTo_name, arr};
    jsonArrayParse(arrObj);
  }
}
void jsonArrayTask(void *nullparam)
{
  uint32_t ptr;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      structTypenamespace::notifyJsonArray_t arrObj = *(structTypenamespace::notifyJsonArray_t *)ptr;
      jsonArrayParse(arrObj);
      // ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void stdStringTask(void *nullparam)
{
  uint32_t ptr;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      structTypenamespace::notifyString_t strObj = *(structTypenamespace::notifyString_t *)ptr;
      stdStringParse(strObj);
      // ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void server_esp(void)
{
  ESP_LOGV("getFreeHeap", "%zu", ESP.getFreeHeap());
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
void server_serial_callback(void)
{
  String info = state.server_serial->readStringUntil('\n');
  String str = String(info.c_str());
  structTypenamespace::notifyString_t strObj = {get<0>(config.server_serial), str};
  stdStringParse(strObj);
}
void setup(void)
{
  Serial.begin(115200);
  state.configLock = xSemaphoreCreateMutex();
  eg_Handle = xEventGroupCreate();
  server_esp();
  state.server_configFs = new MyFs("./config.json");
  if (!state.server_configFs->file_bool)
  {
    config.server_serial = make_tuple(
        "server_serial",
        115200,
        3,
        1);

    config.server_env = make_tuple(
        String(ESP.getEfuseMac(), HEX),
        "server_serial",
        "",
        "");
  }
  // xEventGroupSetBits(eg_Handle, EGBIG_SERVER_CONFIGFS);
  // xEventGroupWaitBits(eg_Handle, EGBIG_SERVER_CONFIGFS, pdTRUE, pdTRUE, portMAX_DELAY);

  // state.server_net = new MyNet(config.server_net);
  // xEventGroupWaitBits(eg_Handle, EGBIG_SERVER_NET, pdTRUE, pdTRUE, portMAX_DELAY);

  // xTaskCreate(stdStringTask, "stdStringTask", 1024 * 10, NULL, state.taskindex++, &stdStringTaskHandle);
  // xTaskCreate(jsonArrayTask, "jsonArrayTask", 1024 * 10, NULL, state.taskindex++, &jsonArrayTaskHandle);

  // state.server_serial = new MySerial(config.server_serial);
  // state.server_serial->onReceive(server_serial_callback);

  // state.server_dz003_taskParam->sendTo_taskHandle = stdStringTaskHandle;
  // // state.server_dz003_taskParam->config = config.server_dz003;
  // xTaskCreate(dz003namespace::resTask, "server_dz003_Task", 1024 * 6, (void *)state.server_dz003_taskParam, state.taskindex++, &server_dz003TaskHandle);

  // state.server_ybl_taskParam->sendTo_taskHandle = stdStringTaskHandle;
  // // state.server_ybl_taskParam->config = config.server_ybl;
  // xTaskCreate(a7129namespace::yblResTask, "ybResTask", 1024 * 4, (void *)state.server_ybl_taskParam, state.taskindex++, NULL);
  // // unordered_map 对象格式的json字符串

  // vTaskStartScheduler();
  //  vTaskDelete(NULL);
}
void loop(void)
{
  // wsServer->cleanupClients(3);
  // ESP_LOGV("getFreeHeap", "%zu", ESP.getFreeHeap());
  // String s = String(ESP.getEfuseMac(), HEX);
  Serial.println(std::get<0>(config.server_env));
  vTaskDelay(1000);
}