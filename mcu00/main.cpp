#include <tuple>
#include <functional>
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
#define EGBIG_CONFIG (1 << 0)
#define EGBIG_NET (1 << 1)
#define EGBIG_YBL (1 << 2)
#define EGBIG_DZ003 (1 << 3)
struct config_t
{
  std::tuple<String, String, String, String, String> mcu_base;
  std::tuple<String, int, String> mcu_serial;
  MyNet::config_t mcu_net;
  dz003namespace::config_t mcu_dz003;
  a7129namespace::config_t mcu_ybl;

} config;
struct state_t
{
  String macId;
  int taskindex;
  String locIp;
  unsigned int testindex;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  QueueHandle_t sendToQueueHandle;
  QueueHandle_t parseStringQueueHandle;
  // TaskHandle_t parseStringTaskHandle;
  MyFs *mcu_configFs;
  MyNet *mcu_net;
  HardwareSerial *mcu_serial;
  dz003namespace::taskParam_t *mcu_dz003TaskParam;
  a7129namespace::taskParam_t *mcu_yblTaskParam;
} state;
//logname std::get<4>(config.mcu_base)
void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
{
  // EventBits_t bits = xEventGroupWaitBits(state.eg_Handle, EGBIG_NET | EGBIG_NET , pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupClearBits(eg_Handle, EGBIG_NET | BIT_2);
  char *er = (char *)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    state.locIp = ETH.localIP().toString(); //.c_str();
    xEventGroupSetBits(state.eg_Handle, EGBIG_NET);
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    state.locIp = WiFi.localIP().toString(); //.c_str();
    xEventGroupSetBits(state.eg_Handle, EGBIG_NET);
    use = 1;
  }
  // xEventGroupSetBits(state.eg_Handle, EGBIG_CONFIGSUCCESS);
  /* EventBits_t eventBits =xEventGroupWaitBits(state.eg_Handle, EGBIG_CONFIGSUCCESS, pdTRUE, pdTRUE, portMAX_DELAY);
  if ((eventBits & EGBIT_CONFIG_SUCCESS) == EGBIT_CONFIG_SUCCESS) {
  // 执行相应的操作
    }
  */
  // if ((xEventGroupGetBits(state.eg_Handle) & EGBIG_NET) != 0)
  // char *data = eventData ? ((char *)eventData) : ((char *)"");
  // ESP_LOGD("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
  ESP_LOGD("esp_eg_on", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
}
void config_set(JsonObject obj)
{
  if (obj.containsKey("mcu_base"))
  {
    JsonArray mcu_base = obj["mcu_base"].as<JsonArray>();
    config.mcu_base = std::make_tuple(mcu_base[0].as<String>(), mcu_base[1].as<String>(), mcu_base[2].as<String>(), mcu_base[3].as<String>(), mcu_base[4].as<String>());
    // ESP_LOGV("config_set", "%s", std::get<0>(config.mcu_base).c_str());
    // ESP_LOGV("config_set", "%s", std::get<1>(config.mcu_base).c_str());
    // ESP_LOGV("config_set", "%s", std::get<2>(config.mcu_base).c_str());
  }
  if (obj.containsKey("mcu_serial"))
  {
    JsonArray mcu_serial = obj["mcu_serial"].as<JsonArray>();
    config.mcu_serial = std::make_tuple(mcu_serial[0].as<String>(), mcu_serial[1].as<int>(), mcu_serial[2].as<String>());
  }
  if (obj.containsKey("mcu_net"))
  {
    JsonArray mcu_net = obj["mcu_net"].as<JsonArray>();
    std::get<0>(config.mcu_net) = mcu_net[0].as<String>();
    JsonArray mcu_net_ap = mcu_net[1].as<JsonArray>();
    std::get<1>(config.mcu_net) = std::make_tuple(mcu_net_ap[0].as<String>());
    JsonArray mcu_net_sta = mcu_net[2].as<JsonArray>();
    std::get<2>(config.mcu_net) = std::make_tuple(mcu_net_sta[0].as<String>(), mcu_net_sta[1].as<String>());
    std::get<3>(config.mcu_net) = mcu_net[3].as<String>();
  }
  if (obj.containsKey("mcu_dz003"))
  {
    JsonArray mcu_dz003 = obj["mcu_dz003"].as<JsonArray>();
    config.mcu_dz003 = std::make_tuple(mcu_dz003[0].as<String>(), mcu_dz003[1].as<int>(), mcu_dz003[2].as<int>(), mcu_dz003[3].as<int>(), mcu_dz003[4].as<int>(), mcu_dz003[5].as<String>());
  }
  if (obj.containsKey("mcu_ybl"))
  {
    JsonArray mcu_ybl = obj["mcu_ybl"].as<JsonArray>();
    std::get<0>(config.mcu_ybl) = mcu_ybl[0].as<String>();
    JsonArray mcu_ybluseIds = mcu_ybl[1].as<JsonArray>();
    for (int i = 0; i < mcu_ybluseIds.size(); ++i)
    {
      std::get<1>(config.mcu_ybl)[i] = mcu_ybluseIds[i].as<a7129namespace::id_t>();
    }
    std::get<2>(config.mcu_ybl) = mcu_ybl[2].as<String>();
  }
}
void config_get(JsonObject obj)
{
  JsonArray mcu_base = obj.createNestedArray("mcu_base");
  mcu_base.add(std::get<0>(config.mcu_base));
  mcu_base.add(std::get<1>(config.mcu_base));
  mcu_base.add(std::get<2>(config.mcu_base));
  mcu_base.add(std::get<3>(config.mcu_base));
  mcu_base.add(std::get<4>(config.mcu_base));
  JsonArray mcu_serial = obj.createNestedArray("mcu_serial");
  mcu_serial.add(std::get<0>(config.mcu_serial));
  mcu_serial.add(std::get<1>(config.mcu_serial));
  mcu_serial.add(std::get<2>(config.mcu_serial));
  JsonArray mcu_net = obj.createNestedArray("mcu_net");
  mcu_net.add(std::get<0>(config.mcu_net));
  JsonArray muc_net_ap = mcu_net.createNestedArray();
  MyNet::ap_t ap = std::get<1>(config.mcu_net);
  muc_net_ap.add(std::get<0>(ap));
  JsonArray muc_net_sta = mcu_net.createNestedArray();
  MyNet::sta_t sta = std::get<2>(config.mcu_net);
  muc_net_sta.add(std::get<0>(sta));
  muc_net_sta.add(std::get<1>(sta));
  mcu_net.add(std::get<3>(config.mcu_net));
  JsonArray mcu_dz003 = obj.createNestedArray("mcu_dz003");
  mcu_dz003.add(std::get<0>(config.mcu_dz003));
  mcu_dz003.add(std::get<1>(config.mcu_dz003));
  mcu_dz003.add(std::get<2>(config.mcu_dz003));
  mcu_dz003.add(std::get<3>(config.mcu_dz003));
  mcu_dz003.add(std::get<4>(config.mcu_dz003));
  mcu_dz003.add(std::get<5>(config.mcu_dz003));
  JsonArray mcu_ybl = obj.createNestedArray("mcu_ybl");
  mcu_ybl.add(std::get<0>(config.mcu_ybl));
  JsonArray mcu_ybluseIds = mcu_ybl.createNestedArray();
  for (const auto &element : std::get<1>(config.mcu_ybl))
  {
    if (element)
      mcu_ybluseIds.add(element);
  }
  mcu_ybl.add(std::get<2>(config.mcu_ybl));
}
void mcuState_get(JsonArray arr)
{
  uint32_t ulBits = xEventGroupGetBits(state.eg_Handle); // 获取 Event Group 变量当前值
  arr.add(state.macId);
  JsonArray egBit = arr.createNestedArray();
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
  arr.add(state.locIp);
  arr.add(state.taskindex);
}
void parseJsonArray(JsonArray arr, myStruct_t myStruct)
{
  String api = arr[0].as<String>();
  if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
  {
    if (api == "init_get")
    {
      arr.clear();
      arr.add("init_set");
      JsonObject obj = arr.createNestedObject();
      config_get(obj);
      JsonArray mcu_state = obj.createNestedArray("mcu_state");
      mcuState_get(mcu_state);
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
      bool success = state.mcu_configFs->writeFile(obj);
      if (!success)
      {
        return;
      }
    }
    else if (api == "config_fromFile")
    {
      arr.clear();
      arr.add("config_set");
      JsonObject obj = arr.createNestedObject();
      bool success = state.mcu_configFs->readFile(obj);
      if (!success)
      {
        return;
      }
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
      JsonArray mcu_state = obj.createNestedArray("mcu_state");
      mcuState_get(mcu_state);
    }
    else if (api.indexOf("mcu_dz003") > -1)
    {
      if (api == "mcu_dz003.fa_set")
      {
        dz003namespace::fa.set(arr[1].as<bool>());
      }
      else if (api == "mcu_dz003.frequency_set")
      {
        dz003namespace::frequency.set(arr[1].as<bool>());
      }
      else if (api == "mcu_dz003.laba_set")
      {
        dz003namespace::laba.set(arr[1].as<bool>());
      }
      else if (api == "mcu_dz003.deng_set")
      {
        dz003namespace::deng.set(arr[1].as<bool>());
      }
      arr.clear();
      arr[0].set("state_set");
      JsonObject obj = arr.createNestedObject();
      JsonObject data = obj.createNestedObject("mcu_dz003State");
      dz003namespace::getState(data);
    }
    else
    {
      arr[0].set("mcu pass");
      arr[1].set(api);
    }
    xSemaphoreGive(state.configLock);
    serializeJson(arr, myStruct.str);
    // state.mcu_serial->println(myStruct.str);
    if (xQueueSend(state.sendToQueueHandle, &myStruct, 0) != pdPASS)
    {
      ESP_LOGE("parseJsonArray", "sendToQueueHandle is full");
    }
  }
  else
  {
    ESP_LOGE("parseJsonArray", "xSemaphoreTake !=pdFALSE");
  }
}
void sendToTask(void *nullparam)
{
  myStruct_t myStruct;
  for (;;)
  {
    if (xQueueReceive(state.sendToQueueHandle, &myStruct, portMAX_DELAY) == pdPASS)
    {
      if (myStruct.sendTo_name == "mcu_serial")
      {
        state.mcu_serial->println(myStruct.str);
        vTaskDelay(50);
      }
      else
      {
        myStruct.str = String("[\"sendTo_name undefind\",\"") + myStruct.str + String("\"]");
        state.mcu_serial->println(myStruct.str);
        vTaskDelay(50);
      }
    }
    else
    {
      ESP_LOGD("sendToTask", "xQueueReceive != pdPASS");
    }
  }
}
void parseStringTask(void *nullparam)
{
  // uint32_t ptr;
  myStruct_t myStruct;
  for (;;)
  {
    // if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    // {
    //  myStruct = *(structTypenamespace::myJsonArray_t *)ptr;
    if (xQueueReceive(state.parseStringQueueHandle, &myStruct, portMAX_DELAY) == pdPASS)
    {
      StaticJsonDocument<2000> doc;
      DeserializationError error = deserializeJson(doc, myStruct.str);
      if (error)
      {
        myStruct.sendTo_name = std::get<4>(config.mcu_base);
        myStruct.str = "[\"parseStringTask error\",\"" + String(error.c_str()) + "\",\"" + myStruct.str + "\"]";
        if (xQueueSend(state.sendToQueueHandle, &myStruct, 0) != pdPASS)
        {
          ESP_LOGD("parseStringTask", "sendToQueueHandle is full");
        }
      }
      else
      {
        myStruct.str.clear();
        JsonArray jsonarray = doc.as<JsonArray>();
        parseJsonArray(jsonarray, myStruct);
      }
    }
    else
    {
      ESP_LOGD("parseStringTask", "parseStringTask xQueueReceive != pdPASS");
    }
  }
}
void mcu_serial_callback(void)
{
  myStruct_t myStruct = {
      .sendTo_name = std::get<0>(config.mcu_serial),
      .str = state.mcu_serial->readStringUntil('\n')};
  if (xQueueSendFromISR(state.parseStringQueueHandle, &myStruct, 0) != pdPASS)
  {
    ESP_LOGE("mcu_serial_callback", "parseStringQueueHandle is full");
  }
}
void initConfig(std::function<void(void)> startCallback)
{

  StaticJsonDocument<2000> doc;
  state.mcu_configFs->readFile(doc);     // 与下行代码交换位置，会不正常
  JsonObject obj = doc.as<JsonObject>(); // 这里用doc.to，也不正常
  // JsonObject obj = doc.to<JsonObject>();
  // state.mcu_configFs->readFile(obj);
  config_set(obj);
  obj["t"] = "readFile config_set";
  obj["testindex"] = state.testindex++;
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());
  startCallback();

  obj.clear();
  config_get(obj);
  obj["t"] = "config_get";
  obj["testindex"] = state.testindex++;
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());

  // ESP_LOGD("getFreeHeap", "%d\n", ESP.getFreeHeap());           // 打印剩余堆内存大小
  // ESP_LOGD("free_heap_size", "%d\n", esp_get_free_heap_size()); // 打印空闲堆内存
  // char *pbuffer = (char *)calloc(1, 2048);
  // printf("--------------- heap:%u ---------------------\r\n", esp_get_free_heap_size());
  // vTaskList(pbuffer);
  // printf("%s", pbuffer);
  // printf("----------------------------------------------\r\n");
  // free(pbuffer);
}

void setup(void)
{
  state.macId = String(ESP.getEfuseMac());
  state.testindex = 0;
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.sendToQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.parseStringQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.mcu_serial = &Serial;
  state.mcu_serial->begin(115200);
  state.mcu_configFs = new MyFs("/config.json");
  // state.mcu_configFs->listFilePrint("/", 3);
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

  initConfig([]()
             { xEventGroupSetBits(state.eg_Handle, EGBIG_CONFIG); });
  xEventGroupWaitBits(state.eg_Handle, EGBIG_CONFIG, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_serial->begin(std::get<1>(config.mcu_serial));
  state.mcu_serial->onReceive(mcu_serial_callback);

  state.mcu_net = new MyNet(config.mcu_net);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_NET, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_yblTaskParam = new a7129namespace::taskParam_t{
      .config = config.mcu_ybl,
      .parseStringQueueHandle = state.parseStringQueueHandle,
      .startCallback = []()
      {
        xEventGroupSetBits(state.eg_Handle, EGBIG_YBL);
      }};
  xTaskCreate(a7129namespace::yblResTask, "mcu_yblTask", 1024 * 6, (void *)state.mcu_yblTaskParam, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_YBL, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_dz003TaskParam = new dz003namespace::taskParam_t{
      .config = config.mcu_dz003,
      .parseStringQueueHandle = state.parseStringQueueHandle,
      .startCallback = []()
      {
        xEventGroupSetBits(state.eg_Handle, EGBIG_DZ003);
      }};
  xTaskCreate(dz003namespace::resTask, "mcu_dz003Task", 1024 * 6, (void *)state.mcu_dz003TaskParam, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_DZ003, pdFALSE, pdTRUE, portMAX_DELAY);

  xTaskCreate(parseStringTask, "parseStringTask", 1024 * 10, NULL, state.taskindex++, NULL);

  xTaskCreate(sendToTask, "sendToTask", 1024 * 4, NULL, state.taskindex++, NULL);

  vTaskDelete(NULL);
}

void loop(void)
{
}