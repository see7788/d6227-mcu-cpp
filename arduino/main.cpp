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
#include <HardwareSerial.h>
#include <structTypenamespace.h>
#include <dz003namespace.h>
#include <a7129namespace.h>
#include <MyFs.h>
#include <MyNet.h>
#include <MyIpc.h>
#define EGBIG_CONFIGSUCCESS (1 << 0)
#define EGBIG_MCU_NET (2 << 0)
using namespace std;
struct config_t
{
  tuple<String, int> serial_mcu;
  tuple<String> log_mcu;
  tuple<String, String, String> const_mcu;
  MyNet::config_t net_mcu;
  dz003namespace::config_t dz003_mcu;
  a7129namespace::config_t ybl_mcu;
} config;
struct state_t
{
  int taskindex;
  dz003namespace::taskParam_t dz003_mcuTaskParam;
  a7129namespace::taskParam_t ybl_mcuTaskParam;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  TaskHandle_t parseStringTaskHandle;
  TaskHandle_t parsejsonArrayTaskHandle;
  String locIp;
  MyFs *configFs_mcu;
  MyNet *net_mcu;
  HardwareSerial *serial_mcu;
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
  if (obj.containsKey("const_mcu"))
  {
    JsonArray const_mcu = obj["const_mcu"].as<JsonArray>();
    config.const_mcu = std::make_tuple(const_mcu[0].as<String>(), const_mcu[1].as<String>(), const_mcu[2].as<String>());
  }
  if (obj.containsKey("log_mcu"))
  {
    JsonArray log_mcu = obj["log_mcu"].as<JsonArray>();
    config.log_mcu = std::make_tuple(log_mcu[0].as<String>());
  }
  if (obj.containsKey("serial_mcu"))
  {
    JsonArray serial_mcu = obj["serial_mcu"].as<JsonArray>();
    config.serial_mcu = std::make_tuple(serial_mcu[0].as<String>(), serial_mcu[1].as<int>());
  }
  if (obj.containsKey("net_mcu"))
  {
    JsonObject net_mcu = obj["net_mcu"].as<JsonObject>();
    config.net_mcu.use = net_mcu["use"].as<String>();
    config.net_mcu.ap = std::make_tuple(net_mcu["ap"][0].as<String>());
    config.net_mcu.sta = std::make_tuple(net_mcu["sta"][0].as<String>(), net_mcu["sta"][1].as<String>());
  }
  if (obj.containsKey("dz003_mcu"))
  {
    JsonArray dz003_mcu = obj["dz003_mcu"].as<JsonArray>();
    config.dz003_mcu = std::make_tuple(dz003_mcu[0].as<String>(), dz003_mcu[1].as<int>(), dz003_mcu[2].as<int>(), dz003_mcu[3].as<int>(), dz003_mcu[4].as<int>());
  }
  if (obj.containsKey("ybl_mcu"))
  {
    JsonArray ybl_mcu = obj["ybl_mcu"].as<JsonArray>();
    get<0>(config.ybl_mcu) = ybl_mcu[0].as<String>();
    JsonArray ybl_mcuuseIds = ybl_mcu[1].as<JsonArray>();
    for (int i = 0; i < ybl_mcuuseIds.size(); ++i)
    {
      get<1>(config.ybl_mcu)[i] = ybl_mcuuseIds[i].as<a7129namespace::id_t>();
    }
  }
}
void config_get(JsonObject &obj)
{
  JsonArray const_mcu = obj.createNestedArray("const_mcu");
  const_mcu.add(get<0>(config.const_mcu));
  const_mcu.add(get<1>(config.const_mcu));
  const_mcu.add(get<2>(config.const_mcu));
  JsonArray log_mcu = obj.createNestedArray("log_mcu");
  log_mcu.add(get<0>(config.log_mcu));
  JsonArray serial_mcu = obj.createNestedArray("serial_mcu");
  serial_mcu.add(get<0>(config.serial_mcu));
  serial_mcu.add(get<1>(config.serial_mcu));
  // net_mcu
  JsonArray dz003_mcu = obj.createNestedArray("dz003_mcu");
  dz003_mcu.add(get<0>(config.dz003_mcu));
  dz003_mcu.add(get<1>(config.dz003_mcu));
  dz003_mcu.add(get<2>(config.dz003_mcu));
  dz003_mcu.add(get<3>(config.dz003_mcu));
  dz003_mcu.add(get<4>(config.dz003_mcu));
  JsonArray ybl_mcu = obj.createNestedArray("ybl_mcu");
  ybl_mcu.add(std::get<0>(config.ybl_mcu));
  JsonArray ybl_mcuuseIds = ybl_mcu.createNestedArray();
  for (const auto &element : std::get<1>(config.ybl_mcu))
  {
    if (element)
      ybl_mcuuseIds.add(element);
  }
  JsonObject net_mcu = obj.createNestedObject("net_mcu");
  net_mcu["use"] = config.net_mcu.use;
  JsonArray muc_net_ap = net_mcu.createNestedArray("ap");
  muc_net_ap.add(get<0>(config.net_mcu.ap));
  JsonArray muc_net_sta = net_mcu.createNestedArray("sta");
  muc_net_sta.add(get<0>(config.net_mcu.sta));
  muc_net_sta.add(get<1>(config.net_mcu.sta));
}
void sendEr(structTypenamespace::notifyString_t &strObj)
{
  ESP_LOGV("DEBUG", "%s", strObj.sendTo_name.c_str());
  if (strObj.sendTo_name == "serial_mcu")
  {
    state.serial_mcu->println(strObj.msg);
  }
  else
  {
    state.serial_mcu->println("[\"sendTo_name undefind\"]");
  }
}
void parseJsonArray(structTypenamespace::notifyJsonArray_t &arrObj)
{
  if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
  {
    JsonArray arr = arrObj.msg;
    String api = arr[0].as<String>();
    // ESP_LOGV("DEBUG", "parseJsonArray:%s", api.c_str());
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
      int success = state.configFs_mcu->writeFile(obj);
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
      state.configFs_mcu->readFile(obj);
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
    ESP_LOGV("DEBUG", "%s", arrObj.sendTo_name.c_str());
    sendEr(strObj);
    xSemaphoreGive(state.configLock);
  }
}
void parsejsonArrayTask(void *nullparam)
{
  uint32_t ptr;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      structTypenamespace::notifyJsonArray_t obj = *(structTypenamespace::notifyJsonArray_t *)ptr;
      parseJsonArray(obj);
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void parseStringTask(void *nullparam)
{
  uint32_t ptr;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      structTypenamespace::notifyString_t obj = *(structTypenamespace::notifyString_t *)ptr;
      StaticJsonDocument<2000> doc;
      DeserializationError error = deserializeJson(doc, obj.msg);
      ESP_LOGV("DEBUG", "%s", obj.sendTo_name.c_str());
      if (error)
      {
        obj.msg = "[\"json pase error\",\"" + String(error.c_str()) + "\",\"" + obj.msg + "\"]";
        sendEr(obj);
      }
      else
      {
        JsonArray arr = doc.as<JsonArray>();
        structTypenamespace::notifyJsonArray_t arrObj = {obj.sendTo_name, arr};
        parseJsonArray(arrObj);
      }
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void espEvent_mcu(void)
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
void serial_mcu_callback(void)
{
  structTypenamespace::notifyString_t *obj = new structTypenamespace::notifyString_t{
      .sendTo_name = get<0>(config.serial_mcu),
      .msg = state.serial_mcu->readStringUntil('\n')};
  ESP_LOGV("DEBUG", "%s", obj->sendTo_name.c_str());
  xTaskNotify(state.parseStringTaskHandle, (uint32_t)obj, eSetValueWithOverwrite);
}
void setup(void)
{
  state.serial_mcu = &Serial;
  state.serial_mcu->begin(115200);
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  espEvent_mcu();
  xTaskCreate(parseStringTask, "parseStringTask", 1024 * 20, NULL, state.taskindex++, &state.parseStringTaskHandle);
  xTaskCreate(parsejsonArrayTask, "parsejsonArrayTask", 1024 * 20, NULL, state.taskindex++, &state.parsejsonArrayTaskHandle);
  state.configFs_mcu = new MyFs("/config.json");
  StaticJsonDocument<2000> doc;
  JsonObject obj = doc.to<JsonObject>();
  state.configFs_mcu->readFile(obj);
  config_set(obj);
  state.serial_mcu->begin(get<1>(config.serial_mcu));
  state.serial_mcu->onReceive(serial_mcu_callback);
  get<0>(config.const_mcu) = String(ESP.getEfuseMac());
  // ESP_LOGV("DEBUG", "----");
  // serializeJson(obj, *state.serial_mcu);
  // ESP_LOGV("DEBUG", "----");
  state.net_mcu = new MyNet(config.net_mcu);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET, pdTRUE, pdTRUE, portMAX_DELAY);
  state.ybl_mcuTaskParam = {
      .config = &config.ybl_mcu,
      //  .sendTo_taskHandle = &state.parseStringTaskHandle
  };
  xTaskCreate(a7129namespace::yblResTask, "ybl_mcuTask", 1024 * 6, (void *)&state.ybl_mcuTaskParam, state.taskindex++, NULL);
  state.dz003_mcuTaskParam = {
      .config = &config.dz003_mcu,
      .sendTo_taskHandle = state.parseStringTaskHandle};
  xTaskCreate(dz003namespace::resTask, "dz003_mcuTask", 1024 * 6, (void *)&state.dz003_mcuTaskParam, state.taskindex++, NULL);
  vTaskStartScheduler();
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
  serializeJson(doc, *state.serial_mcu);
  ESP_LOGV("DEBUG", "==");

  vTaskDelay(3000);
}