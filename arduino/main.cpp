#include <tuple>
#include <ESP.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
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
#include <HardwareSerial.h>
#include <MyWebServer.h>
#include <dz003namespace.h>
#include <MyFs.h>
#include <MyNet.h>
#include <a7129namespace.h>
#define EGBIG_CONFIGSUCCESS (1 << 0)
#define EGBIG_MCU_NET (2 << 0)
using namespace std;
struct config_t
{
  tuple<String, String, String> mcu_const;
  tuple<String> mcu_log;
  tuple<String, int> mcu_serial;
  MyNet::config_t mcu_net;
  tuple<String, int> mcu_ble;
  dz003namespace::config_t mcu_dz003;
  a7129namespace::config_t mcu_ybl;
} config;
struct state_t
{
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  TaskHandle_t stdStringTaskHandle;
  TaskHandle_t jsonArrayTaskHandle;
  String locIp;
  int taskindex;
  dz003namespace::taskParam_t mcu_dz003_taskParam;
  a7129namespace::taskParam_t mcu_ybl_taskParam;
  MyFs *mcu_configFs;
  MyNet *mcu_net;
  HardwareSerial *mcu_serial;
  MyWebServer *mcu_webserver;
} state;
void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
{
  // EventBits_t bits = xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET | EGBIG_MCU_NET , pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupClearBits(eg_Handle, EGBIG_MCU_NET | BIT_2);
  char *er = (char *)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    state.locIp = ETH.localIP().toString().c_str();
    xEventGroupSetBits(state.eg_Handle, EGBIG_MCU_NET);
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    state.locIp = WiFi.localIP().toString().c_str();
    xEventGroupSetBits(state.eg_Handle, EGBIG_MCU_NET);
    use = 1;
  }
  // xEventGroupSetBits(state.eg_Handle, EGBIG_CONFIGSUCCESS);
  /* EventBits_t eventBits =xEventGroupWaitBits(state.eg_Handle, EGBIG_CONFIGSUCCESS, pdTRUE, pdTRUE, portMAX_DELAY);
  if ((eventBits & EGBIT_CONFIG_SUCCESS) == EGBIT_CONFIG_SUCCESS) {
      // 执行相应的操作
    }
  */
  // if ((xEventGroupGetBits(state.eg_Handle) & EGBIG_MCU_NET) != 0)
  // char *data = eventData ? ((char *)eventData) : ((char *)"");
  // ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
  ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
}
void config_set(JsonObject &obj)
{
  if (obj.containsKey("mcu_const"))
  {
    JsonArray mcu_const = obj["mcu_const"].as<JsonArray>();
    config.mcu_const = std::make_tuple(mcu_const[0].as<String>(), mcu_const[1].as<String>(), mcu_const[2].as<String>());
  }
  if (obj.containsKey("mcu_log"))
  {
    JsonArray mcu_log = obj["mcu_log"].as<JsonArray>();
    config.mcu_log = std::make_tuple(mcu_log[0].as<String>());
  }
  if (obj.containsKey("mcu_serial"))
  {
    JsonArray mcu_serial = obj["mcu_serial"].as<JsonArray>();
    config.mcu_serial = std::make_tuple(mcu_serial[0].as<String>(), mcu_serial[1].as<int>());
  }
  if (obj.containsKey("mcu_net"))
  {
    JsonObject mcu_net = obj["mcu_net"].as<JsonObject>();
    config.mcu_net.use = mcu_net["use"].as<String>();
    config.mcu_net.ap = std::make_tuple(mcu_net["ap"][0].as<String>());
    config.mcu_net.sta = std::make_tuple(mcu_net["sta"][0].as<String>(), mcu_net["sta"][1].as<String>());
  }
  if (obj.containsKey("mcu_dz003"))
  {
    JsonArray mcu_dz003 = obj["mcu_dz003"].as<JsonArray>();
    config.mcu_dz003 = std::make_tuple(mcu_dz003[0].as<String>(), mcu_dz003[1].as<int>(), mcu_dz003[2].as<int>(), mcu_dz003[3].as<int>(), mcu_dz003[4].as<int>());
  }
  if (obj.containsKey("mcu_ybl"))
  {
    JsonArray mcu_ybl = obj["mcu_ybl"].as<JsonArray>();
    get<0>(config.mcu_ybl) = mcu_ybl[0].as<String>();
    JsonArray mcu_ybluseIds = mcu_ybl[1].as<JsonArray>();
    for (int i = 0; i < mcu_ybluseIds.size(); ++i)
    {
      get<1>(config.mcu_ybl)[i] = mcu_ybluseIds[i].as<unsigned long>();
    }
  }
}
void config_get(JsonObject &obj)
{
  JsonArray mcu_const = obj.createNestedArray("mcu_const");
  mcu_const.add(get<0>(config.mcu_const));
  mcu_const.add(get<1>(config.mcu_const));
  mcu_const.add(get<2>(config.mcu_const));
  JsonArray mcu_log = obj.createNestedArray("mcu_log");
  mcu_log.add(get<0>(config.mcu_log));
  JsonArray mcu_serial = obj.createNestedArray("mcu_serial");
  mcu_serial.add(get<0>(config.mcu_serial));
  mcu_serial.add(get<1>(config.mcu_serial));
  // mcu_net
  JsonArray mcu_dz003 = obj.createNestedArray("mcu_dz003");
  mcu_dz003.add(get<0>(config.mcu_dz003));
  mcu_dz003.add(get<1>(config.mcu_dz003));
  mcu_dz003.add(get<2>(config.mcu_dz003));
  mcu_dz003.add(get<3>(config.mcu_dz003));
  mcu_dz003.add(get<4>(config.mcu_dz003));
  JsonArray mcu_ybl = obj.createNestedArray("mcu_ybl");
  mcu_ybl.add(std::get<0>(config.mcu_ybl));
  JsonArray mcu_ybluseIds = mcu_ybl.createNestedArray();
  for (const auto &element : std::get<1>(config.mcu_ybl))
  {
    if (element)
      mcu_ybluseIds.add(element);
  }
  JsonObject mcu_net = obj.createNestedObject("mcu_net");
  mcu_net["use"] = config.mcu_net.use;
  JsonArray muc_net_ap = mcu_net.createNestedArray("ap");
  muc_net_ap.add(get<0>(config.mcu_net.ap));
  JsonArray muc_net_sta = mcu_net.createNestedArray("sta");
  muc_net_sta.add(get<0>(config.mcu_net.sta));
  muc_net_sta.add(get<1>(config.mcu_net.sta));
}
void sendEr(structTypenamespace::notifyString_t &strObj)
{
  if (strObj.sendTo_name == "mcu_serial")
  {
    state.mcu_serial->println(strObj.msg);
  }
  else
  {
    strObj.msg = "[\"sendTo_name undefind\"]";
    state.mcu_serial->println(strObj.msg);
  }
  ESP_LOGV("DEBUG", "==");
}
void jsonArrayParse(structTypenamespace::notifyJsonArray_t &arrObj)
{
  if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
  {
    JsonArray arr = arrObj.msg;
    String api = arr[0].as<String>();
    // ESP_LOGV("DEBUG", "jsonArrayParse:%s", api.c_str());
    if (api == "config_set")
    {
      JsonObject obj = arr[1].as<JsonObject>();
      config_set(obj);
    }
    else if (api == "config_get")
    {
      arr.clear();
      arr.add("config_set");
      JsonObject obj = arr.createNestedObject();
      config_get(obj);
    }
    else if (api == "config_toFile")
    {
      arr.clear();
      arr.add("config_set");
      JsonObject obj = arr.createNestedObject();
      config_get(obj);
      int success = state.mcu_configFs->writeFile(obj);
      if (success < 1)
      {
        ESP_LOGE("writeFile", "error");
      }
    }
    else if (api == "config_fromFile")
    {
      arr.clear();
      arr.add("config_set");
      JsonObject obj = arr.createNestedObject();
      state.mcu_configFs->readFile(obj);
      config_set(obj);
    }
    else if (api == "restart")
    {
      ESP.restart();
    }
    else if (api == "state_get")
    {
      uint32_t ulBits = xEventGroupGetBits(state.eg_Handle); // 获取 Event Group 变量当前值
      arr.clear();
      arr.add("state_set");
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
        arr.clear();
        arr[0].set("dz003.State");
        JsonObject obj = arr.createNestedObject();
        dz003namespace::state(obj);
      };
      if (api == "dz003.State")
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
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, strObj.msg);
  if (error)
  {
    strObj.msg = "[\"json pase error\",\"" + String(error.c_str()) + "\"]";
    sendEr(strObj);
  }
  else
  {
    // ESP_LOGV("DEBUG", "stdStringParse%s", strObj.msg.c_str());
    JsonArray arr = doc.as<JsonArray>();
    // ESP_LOGV("DEBUG", "===========\n\n(");
    //  serializeJson(doc, *state.mcu_serial);
    //  state.mcu_serial->println(")");
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
void mcu_esp(void)
{

  ESP_ERROR_CHECK(esp_task_wdt_init(20000, false)); // 初始化看门狗
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  ESP_ERROR_CHECK(esp_task_wdt_status(NULL));
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
}
void mcu_serial_callback(void)
{
  String info = state.mcu_serial->readStringUntil('\n');
  String str = String(info.c_str());
  structTypenamespace::notifyString_t strObj = {get<0>(config.mcu_serial), str};
  stdStringParse(strObj);
}

void setup(void)
{
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.mcu_serial = &Serial;
  state.mcu_serial->begin(115200);
  mcu_esp();
  state.mcu_configFs = new MyFs("/config.json");
  StaticJsonDocument<2000> doc;
  JsonObject obj = doc.to<JsonObject>();
  state.mcu_configFs->readFile(obj);
  config_set(obj);
  get<0>(config.mcu_const) = String(ESP.getEfuseMac());
  // serializeJson(doc, *state.mcu_serial);
  // state.mcu_serial->println("");
  state.mcu_serial->onReceive(mcu_serial_callback);

  state.mcu_net = new MyNet(config.mcu_net);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET, pdTRUE, pdTRUE, portMAX_DELAY);

  xTaskCreate(stdStringTask, "stdStringTask", 1024 * 20, NULL, state.taskindex++, &state.stdStringTaskHandle);
  xTaskCreate(jsonArrayTask, "jsonArrayTask", 1024 * 20, NULL, state.taskindex++, &state.jsonArrayTaskHandle);
  state.mcu_ybl_taskParam = {
      .config = &config.mcu_ybl,
      //  .sendTo_taskHandle = &state.stdStringTaskHandle
  };

  xTaskCreate(a7129namespace::yblResTask, "ybResTask", 1024 * 6, (void *)&state.mcu_ybl_taskParam, state.taskindex++, NULL);
  state.mcu_dz003_taskParam = {
      .config = &config.mcu_dz003,
      .sendTo_taskHandle = state.stdStringTaskHandle};
  xTaskCreate(dz003namespace::resTask, "mcu_dz003_Task", 1024 * 6, (void *)&state.mcu_dz003_taskParam, state.taskindex++, NULL);
  // vTaskStartScheduler();
  ESP_LOGV("DEBUG", "=====================success===========================");
  vTaskDelete(NULL);
}
void loop(void)
{

  // int freeHeap = ESP.getFreeHeap();        // 获取剩余堆内存大小
  // ESP_LOGV("getFreeHeap", "%d", freeHeap); // 打印剩余堆内存大小
  // wsServer->cleanupClients(3);
  // String s = String(ESP.getEfuseMac(), HEX);
  StaticJsonDocument<2000> doc;
  JsonObject obj = doc.to<JsonObject>();
  config_get(obj);
  serializeJson(doc, *state.mcu_serial);
  ESP_LOGV("DEBUG", "==");

  vTaskDelay(3000);
}