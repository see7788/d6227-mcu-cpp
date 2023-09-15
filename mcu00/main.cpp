#include <tuple>
#include <stdio.h>
#include <stdlib.h>
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
// #define EGBIG_parseSend (1 << 1)
using namespace std;
struct config_t
{
  tuple<String, int> mcu00_serial;
  tuple<String> mcu00_log;
  MyNet::config_t mcu00_net;
  dz003namespace::config_t mcu00_dz003;
  a7129namespace::config_t mcu00_ybl;
} config;
struct state_t
{
  int taskindex;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  QueueHandle_t stringQueueHandle;
  TaskHandle_t parseStringTaskHandle;
  TaskHandle_t parsejsonArrayTaskHandle;
  String locIp;
  String macId;
  String packageId;
  MyFs *mcu00_configFs;
  MyNet *mcu00_net;
  HardwareSerial *mcu00_serial;
  dz003namespace::taskParam_t *mcu00_dz003TaskParam;
  a7129namespace::taskParam_t *mcu00_yblTaskParam;
} state;
void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
{
  // EventBits_t bits = xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET | EGBIG_MCU_NET , pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupClearBits(eg_Handle, EGBIG_MCU_NET | BIT_2);
  char *er = (char *)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    state.locIp = ETH.localIP().toString(); //.c_str();
    xEventGroupSetBits(state.eg_Handle, EGBIG_MCU_NET);
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    state.locIp = WiFi.localIP().toString(); //.c_str();
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
  if (obj.containsKey("mcu00_log"))
  {
    JsonArray mcu00_log = obj["mcu00_log"].as<JsonArray>();
    config.mcu00_log = std::make_tuple(mcu00_log[0].as<String>());
  }
  if (obj.containsKey("mcu00_serial"))
  {
    JsonArray mcu00_serial = obj["mcu00_serial"].as<JsonArray>();
    config.mcu00_serial = std::make_tuple(mcu00_serial[0].as<String>(), mcu00_serial[1].as<int>());
  }
  if (obj.containsKey("mcu00_net"))
  {
    JsonObject mcu00_net = obj["mcu00_net"].as<JsonObject>();
    config.mcu00_net.use = mcu00_net["use"].as<String>();
    config.mcu00_net.ap = std::make_tuple(mcu00_net["ap"][0].as<String>());
    config.mcu00_net.sta = std::make_tuple(mcu00_net["sta"][0].as<String>(), mcu00_net["sta"][1].as<String>());
  }
  if (obj.containsKey("mcu00_dz003"))
  {
    JsonArray mcu00_dz003 = obj["mcu00_dz003"].as<JsonArray>();
    config.mcu00_dz003 = std::make_tuple(mcu00_dz003[0].as<String>(), mcu00_dz003[1].as<int>(), mcu00_dz003[2].as<int>(), mcu00_dz003[3].as<int>(), mcu00_dz003[4].as<int>());
  }
  if (obj.containsKey("mcu00_ybl"))
  {
    JsonArray mcu00_ybl = obj["mcu00_ybl"].as<JsonArray>();
    get<0>(config.mcu00_ybl) = mcu00_ybl[0].as<String>();
    JsonArray mcu00_ybluseIds = mcu00_ybl[1].as<JsonArray>();
    for (int i = 0; i < mcu00_ybluseIds.size(); ++i)
    {
      get<1>(config.mcu00_ybl)[i] = mcu00_ybluseIds[i].as<a7129namespace::id_t>();
    }
  }
}
void config_get(JsonObject &obj)
{
  JsonArray mcu00_log = obj.createNestedArray("mcu00_log");
  mcu00_log.add(get<0>(config.mcu00_log));
  JsonArray mcu00_serial = obj.createNestedArray("mcu00_serial");
  mcu00_serial.add(get<0>(config.mcu00_serial));
  mcu00_serial.add(get<1>(config.mcu00_serial));
  // mcu00_net
  JsonArray mcu00_dz003 = obj.createNestedArray("mcu00_dz003");
  mcu00_dz003.add(get<0>(config.mcu00_dz003));
  mcu00_dz003.add(get<1>(config.mcu00_dz003));
  mcu00_dz003.add(get<2>(config.mcu00_dz003));
  mcu00_dz003.add(get<3>(config.mcu00_dz003));
  mcu00_dz003.add(get<4>(config.mcu00_dz003));
  JsonArray mcu00_ybl = obj.createNestedArray("mcu00_ybl");
  mcu00_ybl.add(std::get<0>(config.mcu00_ybl));
  JsonArray mcu00_ybluseIds = mcu00_ybl.createNestedArray();
  for (const auto &element : std::get<1>(config.mcu00_ybl))
  {
    if (element)
      mcu00_ybluseIds.add(element);
  }
  JsonObject mcu00_net = obj.createNestedObject("mcu00_net");
  mcu00_net["use"] = config.mcu00_net.use;
  JsonArray muc_net_ap = mcu00_net.createNestedArray("ap");
  muc_net_ap.add(get<0>(config.mcu00_net.ap));
  JsonArray muc_net_sta = mcu00_net.createNestedArray("sta");
  muc_net_sta.add(get<0>(config.mcu00_net.sta));
  muc_net_sta.add(get<1>(config.mcu00_net.sta));
}
void sendLog(String &jsonstr)
{
  String sendTo = get<0>(config.mcu00_log);
  if (sendTo == "mcu00_serial")
  {
    state.mcu00_serial->println(jsonstr);
  }
  else
  {
    state.mcu00_serial->println("[\"sendTo_name undefind\"]");
  }
}
void sendEr(structTypenamespace::myString_t &strObj)
{
  // ESP_LOGV("DEBUG", "%s", strObj.sendTo_name.c_str());
  if (strObj.sendTo_name == "mcu00_serial")
  {
    state.mcu00_serial->println(strObj.msg);
  }
  else
  {
    String msg = "[\"sendTo_name undefind\"]";
    sendLog(msg);
  }
}
void parseJsonArray(structTypenamespace::myJsonArray_t &arrObj)
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
      int success = state.mcu00_configFs->writeFile(obj);
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
      state.mcu00_configFs->readFile(obj);
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
        JsonObject data = obj.createNestedObject("mcu00_dz003_state");
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
    structTypenamespace::myString_t strObj = {
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
  structTypenamespace::myJsonArray_t obj;
  for (;;)
  {
    if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    {
      obj = *(structTypenamespace::myJsonArray_t *)ptr;
      parseJsonArray(obj);
      ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
    }
  }
}
void parseStringTask(void *nullparam)
{
  uint32_t ptr;
  structTypenamespace::myString_t obj;
  while (1)
  {
    // if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    //     {
    //       obj = *(structTypenamespace::myString_t *)ptr;
    if (xQueueReceive(state.stringQueueHandle, &obj, portMAX_DELAY) == pdPASS)
    {
      StaticJsonDocument<2000> doc;
      DeserializationError error = deserializeJson(doc, obj.msg);
      if (error)
      {
        String msg = "[\"json pase error\",\"" + String(error.c_str()) + "\",\"" + obj.msg + "\"]";
        sendLog(msg);
      }
      else
      {
        JsonArray arr = doc.as<JsonArray>();
        structTypenamespace::myJsonArray_t arrObj = {
            .sendTo_name = obj.sendTo_name,
            .msg = arr};
        parseJsonArray(arrObj);
      }
    }
  }
}
void mcu00_serial_callback(void)
{
  structTypenamespace::myString_t *obj = new structTypenamespace::myString_t{
      .sendTo_name = get<0>(config.mcu00_serial),
      .msg = state.mcu00_serial->readStringUntil('\n')};
  // ESP_LOGV("DEBUG", "%s", obj->sendTo_name.c_str());
  // xTaskNotify(state.parseStringTaskHandle, (uint32_t)obj, eSetValueWithOverwrite);
  if (xQueueSend(state.stringQueueHandle, obj, 500) != pdPASS)
  {
    ESP_LOGV("DEBUG", "Queue is full");
  }
}
void setup(void)
{
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.stringQueueHandle = xQueueCreate(8, sizeof(structTypenamespace::myString_t));
  state.macId = String(ESP.getEfuseMac());
  state.packageId = "6227";
  state.mcu00_serial = &Serial;
  state.mcu00_serial->begin(115200);

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
  ESP_LOGV("DEBUG", "=======esp_event_handler_register success=========");

  state.mcu00_configFs = new MyFs("/config.json");
  StaticJsonDocument<2000> doc;
  state.mcu00_configFs->readFile(doc);
  JsonObject obj = doc.as<JsonObject>();
  config_set(obj);
  ESP_LOGV("DEBUG", "========mcu00_configFs");
  serializeJson(doc, *state.mcu00_serial);
  state.mcu00_serial->begin(get<1>(config.mcu00_serial));
  state.mcu00_serial->onReceive(mcu00_serial_callback);
  ESP_LOGV("DEBUG", "=========mcu00_serial");

  xTaskCreate(parseStringTask, "parseStringTask", 1024 * 20, NULL, state.taskindex++, &state.parseStringTaskHandle);
  xTaskCreate(parsejsonArrayTask, "parsejsonArrayTask", 1024 * 20, NULL, state.taskindex++, &state.parsejsonArrayTaskHandle);
  ESP_LOGV("DEBUG", "============parseTask");
  state.mcu00_net = new MyNet(config.mcu00_net);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET, pdTRUE, pdTRUE, portMAX_DELAY);
  structTypenamespace::myString_t notify1 = {
      .sendTo_name = get<0>(config.mcu00_log),
      .msg = "[\"config_get\"]"};
  xQueueSend(state.stringQueueHandle, &notify1, eSetValueWithOverwrite);
  structTypenamespace::myString_t notify2 = {
      .sendTo_name = get<0>(config.mcu00_log),
      .msg = "[\"state_get\"]"};
  xQueueSend(state.stringQueueHandle, &notify2, eSetValueWithOverwrite);
  // ESP_LOGV("DEBUG", "==============init_get");
  state.mcu00_yblTaskParam = new a7129namespace::taskParam_t{
      .config = config.mcu00_ybl,
      // .xTaskNotifyWait_taskHandle = state.parseStringTaskHandle,
      .stringQueueHandle = state.stringQueueHandle};
  xTaskCreate(a7129namespace::yblResTask, "mcu00_yblTask", 1024 * 6, (void *)state.mcu00_yblTaskParam, state.taskindex++, NULL);

  state.mcu00_dz003TaskParam = new dz003namespace::taskParam_t{
      .config = config.mcu00_dz003,
      //  .notifyWait_taskHandle = state.parseStringTaskHandle,
      .stringQueueHandle = state.stringQueueHandle};
  xTaskCreate(dz003namespace::resTask, "mcu00_dz003Task", 1024 * 6, (void *)state.mcu00_dz003TaskParam, state.taskindex++, NULL);

  ESP_LOGV("DEBUG", "=========setup success");
  // vTaskStartScheduler();//Arduin内部已经启用
  vTaskDelete(NULL);
}
void loop(void)
{

  // int freeHeap = ESP.getFreeHeap();        // 获取剩余堆内存大小
  // ESP_LOGV("getFreeHeap", "%d", freeHeap); // 打印剩余堆内存大小
  // wsServer->cleanupClients(3);
  // String s = String(ESP.getEfuseMac(), HEX);
  // StaticJsonDocument<2000> doc;
  // JsonObject obj = doc.as<JsonObject>();
  // config_get(obj);
  // ESP_LOGV("DEBUG", "==");
  // serializeJson(doc, *state.mcu00_serial);
  // char *pbuffer = (char *)calloc(1, 2048);
  // printf("--------------- heap:%u ---------------------\r\n", esp_get_free_heap_size());
  // vTaskList(pbuffer);
  // printf("%s", pbuffer);
  // printf("----------------------------------------------\r\n");
  // free(pbuffer);
  vTaskDelay(3000);
}
