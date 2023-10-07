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
#include <myStruct_t.h>
#include <dz003namespace.h>
#include <a7129namespace.h>
#include <MyFs.h>
#include <MyNet.h>
#include <MyIpc.h>
#define EGBIG_MCU_NET (1 << 0)
// #define EGBIG_parseSend (1 << 1)
struct config_t
{
  std::tuple<String, int> mcu00_serial;
  std::tuple<String> mcu00_log;
  MyNet::config_t mcu00_net;
  dz003namespace::config_t mcu00_dz003;
  a7129namespace::config_t mcu00_ybl;
} config;
struct state_t
{
  int taskindex;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  QueueHandle_t parseStringQueueHandle;
  QueueHandle_t parsejsonArrayQueueHandle;
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
    JsonArray mcu00_net = obj["mcu00_net"].as<JsonArray>();
    std::get<0>(config.mcu00_net) = mcu00_net[0].as<String>();
    JsonArray mcu00_net_ap = mcu00_net[1].as<JsonArray>();
    std::get<1>(config.mcu00_net) = std::make_tuple(mcu00_net_ap[0].as<String>());
    JsonArray mcu00_net_sta = mcu00_net[2].as<JsonArray>();
    std::get<2>(config.mcu00_net) = std::make_tuple(mcu00_net_sta[0].as<String>(), mcu00_net_sta[1].as<String>());
  }
  if (obj.containsKey("mcu00_dz003"))
  {
    JsonArray mcu00_dz003 = obj["mcu00_dz003"].as<JsonArray>();
    config.mcu00_dz003 = std::make_tuple(mcu00_dz003[0].as<String>(), mcu00_dz003[1].as<int>(), mcu00_dz003[2].as<int>(), mcu00_dz003[3].as<int>(), mcu00_dz003[4].as<int>());
  }
  if (obj.containsKey("mcu00_ybl"))
  {
    JsonArray mcu00_ybl = obj["mcu00_ybl"].as<JsonArray>();
    std::get<0>(config.mcu00_ybl) = mcu00_ybl[0].as<String>();
    JsonArray mcu00_ybluseIds = mcu00_ybl[1].as<JsonArray>();
    for (int i = 0; i < mcu00_ybluseIds.size(); ++i)
    {
      std::get<1>(config.mcu00_ybl)[i] = mcu00_ybluseIds[i].as<a7129namespace::id_t>();
    }
  }
}
void config_get(JsonObject &obj)
{
  JsonArray mcu00_log = obj.createNestedArray("mcu00_log");
  mcu00_log.add(std::get<0>(config.mcu00_log));
  JsonArray mcu00_serial = obj.createNestedArray("mcu00_serial");
  mcu00_serial.add(std::get<0>(config.mcu00_serial));
  mcu00_serial.add(std::get<1>(config.mcu00_serial));
  JsonArray mcu00_net = obj.createNestedArray("mcu00_net");
  MyNet::type_t uset;
  MyNet::ap_t ap;
  MyNet::sta_t sta;
  std::tie(uset, ap, sta) = config.mcu00_net;
  mcu00_net.add(uset);
  JsonArray muc_net_ap = mcu00_net.createNestedArray();
  muc_net_ap.add(std::get<0>(ap));
  JsonArray muc_net_sta = mcu00_net.createNestedArray();
  muc_net_sta.add(std::get<0>(sta));
  muc_net_sta.add(std::get<1>(sta));
  JsonArray mcu00_dz003 = obj.createNestedArray("mcu00_dz003");
  mcu00_dz003.add(std::get<0>(config.mcu00_dz003));
  mcu00_dz003.add(std::get<1>(config.mcu00_dz003));
  mcu00_dz003.add(std::get<2>(config.mcu00_dz003));
  mcu00_dz003.add(std::get<3>(config.mcu00_dz003));
  mcu00_dz003.add(std::get<4>(config.mcu00_dz003));
  JsonArray mcu00_ybl = obj.createNestedArray("mcu00_ybl");
  mcu00_ybl.add(std::get<0>(config.mcu00_ybl));
  JsonArray mcu00_ybluseIds = mcu00_ybl.createNestedArray();
  for (const auto &element : std::get<1>(config.mcu00_ybl))
  {
    if (element)
      mcu00_ybluseIds.add(element);
  }
}
void sendLog(String &jsonstr)
{
  String sendTo = std::get<0>(config.mcu00_log);
  if (sendTo == "mcu00_serial")
  {
    state.mcu00_serial->println(jsonstr);
  }
  else
  {
    state.mcu00_serial->println("[\"sendTo_name undefind\"]");
  }
}
void dz003State_get(JsonArray &arr)
{
  arr.clear();
  arr[0].set("state_set");
  JsonObject obj = arr.createNestedObject();
  JsonObject data = obj.createNestedObject("mcu00_dz003State");
  dz003namespace::state(data);
};
void mcuState_get(JsonObject &obj)
{
  uint32_t ulBits = xEventGroupGetBits(state.eg_Handle); // 获取 Event Group 变量当前值
  JsonArray egBit = obj.createNestedArray("egBit");
  for (int i = sizeof(ulBits) * 8 - 1; i >= 0; i--)
  { // 循环输出每个二进制位
    uint32_t mask = 1 << i;
    if (ulBits & mask)
    {
      egBit.add(true);
    }
    else
    {
      egBit.add(false);
    }
  }
  obj["taskindex"] = state.taskindex;
  obj["locIp"] = state.locIp;
  obj["macId"] = state.macId;
  obj["packageId"] = state.packageId;
}
void parseJsonArray(myStruct_t &myStruct)
{
  String msg;
  JsonArray arr = myStruct.jsonArr;
  String api = arr[0].as<String>();
  // ESP_LOGV("DEBUG", "parseJsonArray:%s", api.c_str());
  if (api == "init_get")
  {
    arr.clear();
    arr.add("init_set");
    JsonObject obj = arr.createNestedObject();
    config_get(obj);
    JsonObject mcu00_state = obj.createNestedObject("mcu00_state");
    mcuState_get(mcu00_state);
  }
  else if (api == "config_set")
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
    arr.clear();
    arr.add("state_set");
    JsonObject obj = arr.createNestedObject();
    JsonObject mcu00_state = obj.createNestedObject("mcu00_state");
    mcuState_get(mcu00_state);
  }
  else if (api == "dz003.State")
  {
    dz003State_get(arr);
  }
  else if (api == "dz003.fa_set")
  {
    dz003namespace::fa_set(arr[1].as<bool>());
    dz003State_get(arr);
  }
  else if (api == "dz003.frequency_set")
  {
    dz003namespace::frequency_set(arr[1].as<bool>());
    dz003State_get(arr);
  }
  else if (api == "dz003.laba_set")
  {
    dz003namespace::laba_set(arr[1].as<bool>());
    dz003State_get(arr);
  }
  else if (api == "dz003.deng_set")
  {
    dz003namespace::deng_set(arr[1].as<bool>());
    dz003State_get(arr);
  }
  else
  {
    arr[0].set("mcu pass");
  }
  serializeJson(arr, msg);
  if (myStruct.sendTo_name == "mcu00_serial")
  {
    state.mcu00_serial->println(msg);
  }
  else
  {
    msg = "[\"parseJsonArray sendTo_name undefind\"]";
    sendLog(msg);
  }
}

void parsejsonArrayTask(void *nullparam)
{
  uint32_t ptr;
  myStruct_t myStruct;
  for (;;)
  {
    // if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    // {
    //  myStruct = *(structTypenamespace::myJsonArray_t *)ptr;
    if (xQueueReceive(state.parsejsonArrayQueueHandle, &myStruct, portMAX_DELAY) == pdPASS)
    {
      // ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
      if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
      {
        parseJsonArray(myStruct);
        xSemaphoreGive(state.configLock);
      }
      else
      {
        ESP_LOGE("DEBUG", "xSemaphoreTake !=pdFALSE");
      }
    }
    else
    {
      ESP_LOGV("DEBUG", "parsejsonArrayTask xQueueReceive != pdPASS");
    }
  }
}
void parseStringTask(void *nullparam)
{
  uint32_t ptr;
  myStruct_t myStruct;
  for (;;)
  {
    if (xQueueReceive(state.parseStringQueueHandle, &myStruct, portMAX_DELAY) == pdPASS)
    {
      StaticJsonDocument<2000> doc;
      DeserializationError error = deserializeJson(doc, myStruct.str);
      if (error)
      {
        String msg = "[\"parseStringTask error\",\"" + String(error.c_str()) + "\",\"" + myStruct.str + "\"]";
        sendLog(msg);
      }
      else
      {
        myStruct.str.clear();
        myStruct.jsonArr = doc.as<JsonArray>();
        if (xQueueSend(state.parsejsonArrayQueueHandle, &myStruct, 0) != pdPASS)
        {
          ESP_LOGV("DEBUG", "parseStringQueueHandle is full");
        }
      }
    }
    else
    {
      ESP_LOGV("DEBUG", "parseStringTask xQueueReceive != pdPASS");
    }
  }
}
void mcu00_serial_callback(void)
{
  myStruct_t myStruct = {
      .sendTo_name = std::get<0>(config.mcu00_serial),
      .str = state.mcu00_serial->readStringUntil('\n')};
  if (xQueueSend(state.parseStringQueueHandle, &myStruct, 0) != pdPASS)
  {
    ESP_LOGV("DEBUG", "parseStringQueueHandle is full");
  }
}
void setup(void)
{
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.parseStringQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.parsejsonArrayQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.macId = String(ESP.getEfuseMac());
  state.packageId = "6227";
  state.mcu00_serial = &Serial;
  state.mcu00_serial->begin(115200);
  // state.mcu00_serial->setRxBufferSize(1024);
  // state.mcu00_serial->setTxBufferSize(1024);

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
  DynamicJsonDocument doc(2000);
  state.mcu00_configFs->readFile(doc);
  JsonObject obj = doc.as<JsonObject>();
  config_set(obj);
  obj["t"] = "config_set";
  serializeJson(doc, *state.mcu00_serial);
  state.mcu00_serial->println("");
  obj.clear();
  config_get(obj);
  obj["t"] = "config_get";
  serializeJson(doc, *state.mcu00_serial);
  state.mcu00_serial->println("");
  state.mcu00_serial->begin(std::get<1>(config.mcu00_serial));
  state.mcu00_serial->onReceive(mcu00_serial_callback);
  ESP_LOGV("DEBUG", "=========mcu00_serial");

  state.mcu00_net = new MyNet(config.mcu00_net);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_MCU_NET, pdTRUE, pdTRUE, portMAX_DELAY);
  myStruct_t notify1 = {
      .sendTo_name = std::get<0>(config.mcu00_log),
      .str = "[\"config_get\"]"};
  xQueueSend(state.parseStringQueueHandle, &notify1, eSetValueWithOverwrite);
  myStruct_t notify2 = {
      .sendTo_name = std::get<0>(config.mcu00_log),
      .str = "[\"state_get\"]"};
  xQueueSend(state.parseStringQueueHandle, &notify2, eSetValueWithOverwrite);
  state.mcu00_yblTaskParam = new a7129namespace::taskParam_t{
      .config = config.mcu00_ybl,
      .parseStringQueueHandle = state.parseStringQueueHandle};
  xTaskCreate(a7129namespace::yblResTask, "mcu00_yblTask", 1024 * 6, (void *)state.mcu00_yblTaskParam, state.taskindex++, NULL);

  state.mcu00_dz003TaskParam = new dz003namespace::taskParam_t{
      .config = config.mcu00_dz003,
      .parseStringQueueHandle = state.parseStringQueueHandle};
  xTaskCreate(dz003namespace::resTask, "mcu00_dz003Task", 1024 * 6, (void *)state.mcu00_dz003TaskParam, state.taskindex++, NULL);

  xTaskCreate(parseStringTask, "parseStringTask", 1024 * 20, NULL, state.taskindex++, &state.parseStringTaskHandle);
  xTaskCreate(parsejsonArrayTask, "parsejsonArrayTask", 1024 * 20, NULL, state.taskindex++, &state.parsejsonArrayTaskHandle);

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
