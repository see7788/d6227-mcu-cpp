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
#define EGBIG_MCU_NET (1 << 0)
using namespace std;
struct config_t
{
  tuple<String, int> serial_mcu00;
  tuple<String> log_mcu00;
  MyNet::config_t net_mcu00;
  dz003namespace::config_t dz003_mcu00;
  a7129namespace::config_t ybl_mcu00;
} config;
struct state_t
{
  int taskindex;
  dz003namespace::taskParam_t dz003_mcu00TaskParam;
  a7129namespace::taskParam_t ybl_mcu00TaskParam;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  TaskHandle_t parseStringTaskHandle;
  TaskHandle_t parsejsonArrayTaskHandle;
  String locIp;
  String macId;
  String packageId;
  MyFs *configFs_mcu;
  MyNet *net_mcu00;
  HardwareSerial *serial_mcu00;
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
  if (obj.containsKey("log_mcu00"))
  {
    JsonArray log_mcu00 = obj["log_mcu00"].as<JsonArray>();
    config.log_mcu00 = std::make_tuple(log_mcu00[0].as<String>());
  }
  if (obj.containsKey("serial_mcu00"))
  {
    JsonArray serial_mcu00 = obj["serial_mcu00"].as<JsonArray>();
    config.serial_mcu00 = std::make_tuple(serial_mcu00[0].as<String>(), serial_mcu00[1].as<int>());
  }
  if (obj.containsKey("net_mcu00"))
  {
    JsonObject net_mcu00 = obj["net_mcu00"].as<JsonObject>();
    config.net_mcu00.use = net_mcu00["use"].as<String>();
    config.net_mcu00.ap = std::make_tuple(net_mcu00["ap"][0].as<String>());
    config.net_mcu00.sta = std::make_tuple(net_mcu00["sta"][0].as<String>(), net_mcu00["sta"][1].as<String>());
  }
  if (obj.containsKey("dz003_mcu00"))
  {
    JsonArray dz003_mcu00 = obj["dz003_mcu00"].as<JsonArray>();
    config.dz003_mcu00 = std::make_tuple(dz003_mcu00[0].as<String>(), dz003_mcu00[1].as<int>(), dz003_mcu00[2].as<int>(), dz003_mcu00[3].as<int>(), dz003_mcu00[4].as<int>());
  }
  if (obj.containsKey("ybl_mcu00"))
  {
    JsonArray ybl_mcu00 = obj["ybl_mcu00"].as<JsonArray>();
    get<0>(config.ybl_mcu00) = ybl_mcu00[0].as<String>();
    JsonArray ybl_mcu00useIds = ybl_mcu00[1].as<JsonArray>();
    for (int i = 0; i < ybl_mcu00useIds.size(); ++i)
    {
      get<1>(config.ybl_mcu00)[i] = ybl_mcu00useIds[i].as<a7129namespace::id_t>();
    }
  }
}
void config_get(JsonObject &obj)
{
  JsonArray log_mcu00 = obj.createNestedArray("log_mcu00");
  log_mcu00.add(get<0>(config.log_mcu00));
  JsonArray serial_mcu00 = obj.createNestedArray("serial_mcu00");
  serial_mcu00.add(get<0>(config.serial_mcu00));
  serial_mcu00.add(get<1>(config.serial_mcu00));
  // net_mcu00
  JsonArray dz003_mcu00 = obj.createNestedArray("dz003_mcu00");
  dz003_mcu00.add(get<0>(config.dz003_mcu00));
  dz003_mcu00.add(get<1>(config.dz003_mcu00));
  dz003_mcu00.add(get<2>(config.dz003_mcu00));
  dz003_mcu00.add(get<3>(config.dz003_mcu00));
  dz003_mcu00.add(get<4>(config.dz003_mcu00));
  JsonArray ybl_mcu00 = obj.createNestedArray("ybl_mcu00");
  ybl_mcu00.add(std::get<0>(config.ybl_mcu00));
  JsonArray ybl_mcu00useIds = ybl_mcu00.createNestedArray();
  for (const auto &element : std::get<1>(config.ybl_mcu00))
  {
    if (element)
      ybl_mcu00useIds.add(element);
  }
  JsonObject net_mcu00 = obj.createNestedObject("net_mcu00");
  net_mcu00["use"] = config.net_mcu00.use;
  JsonArray muc_net_ap = net_mcu00.createNestedArray("ap");
  muc_net_ap.add(get<0>(config.net_mcu00.ap));
  JsonArray muc_net_sta = net_mcu00.createNestedArray("sta");
  muc_net_sta.add(get<0>(config.net_mcu00.sta));
  muc_net_sta.add(get<1>(config.net_mcu00.sta));
}
void sendEr(structTypenamespace::notifyString_t &strObj)
{
  // ESP_LOGV("DEBUG", "%s", strObj.sendTo_name.c_str());
  if (strObj.sendTo_name == "serial_mcu00")
  {
    state.serial_mcu00->println(strObj.msg);
  }
  else
  {
    state.serial_mcu00->println("[\"sendTo_name undefind\"]");
  }
}
void sendLog(String &str)
{
  structTypenamespace::notifyString_t obj = {
      .sendTo_name = get<0>(config.log_mcu00),
      .msg = str};
  sendEr(obj);
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
      JsonObject data = obj.createNestedObject("mcu00");
      JsonObject egBit = data.createNestedObject("egBit");
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
      obj["taskindex"] = state.taskindex;
      obj["locIp"] = state.locIp;
      obj["macId"] = state.macId;
      obj["packageId"] = state.packageId;
    }
    else
    {
      auto getdz003State = [&arr]()
      {
        arr.clear();
        arr[0].set("state_set");
        JsonObject obj = arr.createNestedObject();
        JsonObject data = obj.createNestedObject("dz003_mcu00");
        dz003namespace::state(data);
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
    structTypenamespace::notifyString_t strObj = {
        .sendTo_name = arrObj.sendTo_name,
        .msg = ""};
    // ESP_LOGV("DEBUG", "%s", arrObj.sendTo_name.c_str());
    serializeJson(arr, strObj.msg);
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
void serial_mcu00_callback(void)
{
  structTypenamespace::notifyString_t *obj = new structTypenamespace::notifyString_t{
      .sendTo_name = get<0>(config.serial_mcu00),
      .msg = state.serial_mcu00->readStringUntil('\n')};
  // ESP_LOGV("DEBUG", "%s", obj->sendTo_name.c_str());
  xTaskNotify(state.parseStringTaskHandle, (uint32_t)obj, eSetValueWithOverwrite);
}
void setup(void)
{
  state.macId = String(ESP.getEfuseMac());
  state.packageId = "6227";
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.serial_mcu00 = &Serial;
  state.serial_mcu00->begin(115200);

  espEvent_mcu();

  state.configFs_mcu = new MyFs("/config.json");
  StaticJsonDocument<2000> doc;
  state.configFs_mcu->readFile(doc);
  JsonObject obj = doc.as<JsonObject>();
  config_set(obj);
  // serializeJson(doc, *state.serial_mcu00);
  state.serial_mcu00->begin(get<1>(config.serial_mcu00));
  state.serial_mcu00->onReceive(serial_mcu00_callback);

  xTaskCreate(parseStringTask, "parseStringTask", 1024 * 20, NULL, state.taskindex++, &state.parseStringTaskHandle);
  xTaskCreate(parsejsonArrayTask, "parsejsonArrayTask", 1024 * 20, NULL, state.taskindex++, &state.parsejsonArrayTaskHandle);

  structTypenamespace::notifyString_t *notify1 = new structTypenamespace::notifyString_t{
      .sendTo_name = get<0>(config.log_mcu00),
      .msg = "[\"config_get\"]"};
  xTaskNotify(state.parseStringTaskHandle, (uint32_t)notify1, eSetValueWithOverwrite);
  vTaskDelay(500);
  delete notify1;
  structTypenamespace::notifyString_t *notify2 = new structTypenamespace::notifyString_t{
      .sendTo_name = get<0>(config.log_mcu00),
      .msg = "[\"state_get\"]"};
  xTaskNotify(state.parseStringTaskHandle, (uint32_t)notify2, eSetValueWithOverwrite);
  vTaskDelay(500);
  delete notify2;

  state.net_mcu00 = new MyNet(config.net_mcu00);

  xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET, pdTRUE, pdTRUE, portMAX_DELAY);
  state.ybl_mcu00TaskParam = {
      .config = &config.ybl_mcu00,
      //  .sendTo_taskHandle = &state.parseStringTaskHandle
  };
  xTaskCreate(a7129namespace::yblResTask, "ybl_mcu00Task", 1024 * 6, (void *)&state.ybl_mcu00TaskParam, state.taskindex++, NULL);

  state.dz003_mcu00TaskParam = {
      .config = &config.dz003_mcu00,
      .sendTo_taskHandle = state.parseStringTaskHandle};
  xTaskCreate(dz003namespace::resTask, "dz003_mcu00Task", 1024 * 6, (void *)&state.dz003_mcu00TaskParam, state.taskindex++, NULL);

  ESP_LOGV("DEBUG", "=====================setup success===========================");
  // vTaskStartScheduler();//Arduin内部已经启用
  vTaskDelete(NULL);
}
void loop(void)
{

  // int freeHeap = ESP.getFreeHeap();        // 获取剩余堆内存大小
  // ESP_LOGV("getFreeHeap", "%d", freeHeap); // 打印剩余堆内存大小
  // wsServer->cleanupClients(3);
  // String s = String(ESP.getEfuseMac(), HEX);
  StaticJsonDocument<2000> doc;
  JsonObject obj = doc.as<JsonObject>();
  config_get(obj);
  ESP_LOGV("DEBUG", "==");
  serializeJson(doc, *state.serial_mcu00);

  vTaskDelay(3000);
}