#include <ESP.h>
#include <tuple>
#include <functional>
#include <initializer_list>
#include <stdio.h>
#include <stdlib.h>
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
#include <Arduino.h>
#include <IPAddress.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>//硬串口,Software Serial软串口
#include <ESPAsyncWebServer.h>
#include <myStruct_t.h>
#include <dz003namespace.h>
#include <a7129namespace.h>
#include <MyFs.h>
#include <MyNet.h>
#include <MyWebServer.h>
#include <MyBle.h>
#define EGBIG_CONFIGJSON (1 << 0)
#define EGBIG_REQ (1 << 1)
#define EGBIG_RES (1 << 2)
#define EGBIG_NET (1 << 3)
#define EGBIG_YBL (1 << 4)
#define EGBIG_DZ003 (1 << 5)
typedef struct
{
  std::tuple<String, String, String, String> mcu_base;
  std::tuple<String, int, String> mcu_serial;
  MyNet::config_t mcu_net;
  dz003namespace::config_t mcu_dz003;
  a7129namespace::ybl::config_t mcu_ybl;
  MyWebServer::webPageServer_t mcu_webPageServer;
  MyWebServer::wsServer_t mcu_wsServer;
  MyWebServer::esServer_t mcu_esServer;
} config_t;
config_t config;
typedef struct
{
  String macId;
  int taskindex;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  QueueHandle_t reqQueueHandle;
  QueueHandle_t resQueueHandle;
  MyFs* configFs;
  MyFs* i18nFs;
  MyNet* mcu_net;
  HardwareSerial* mcu_serial;
  dz003namespace::mainTaskParam_t* mcu_dz003TaskParam;
  a7129namespace::ybl::taskParam_t* mcu_yblTaskParam;
  MyWebServer* mcu_webServer;
} state_t;
state_t state;
void esp_eg_on(void* registEr, esp_event_base_t postEr, int32_t eventId, void* eventData)
{
  HardwareSerial e();
  // EventBits_t bits = xEventGroupWaitBits(state.eg_Handle, EGBIG_NET | EGBIG_NET , pdFALSE, pdTRUE, portMAX_DELAY);
  // xEventGroupSetBits(state.eg_Handle, EGBIG_CONFIGJSONSUCCESS);
  // xEventGroupClearBits(state.eg_Handle, EGBIG_NET | BIT_2);
  char* er = (char*)registEr;
  int use = 0;
  if (postEr == IP_EVENT && eventId == 4)
  {
    xEventGroupSetBits(state.eg_Handle, EGBIG_NET);
    ESP_LOGV("ETBIG", "EGBIG_NET");
    use = 1;
  }
  else if (postEr == IP_EVENT && eventId == 0)
  {
    xEventGroupSetBits(state.eg_Handle, EGBIG_NET);
    ESP_LOGV("ETBIG", "EGBIG_NET");
    use = 1;
  }
  /* EventBits_t eventBits =xEventGroupWaitBits(state.eg_Handle, EGBIG_CONFIGJSONSUCCESS, pdTRUE, pdTRUE, portMAX_DELAY);
  if ((eventBits & EGBIT_CONFIG_SUCCESS) == EGBIT_CONFIG_SUCCESS) {
  // 执行相应的操作
    }
  */
  // if ((xEventGroupGetBits(state.eg_Handle) & EGBIG_NET) != 0)
  // char *data = eventData ? ((char *)eventData) : ((char *)"");
  // ESP_LOGD("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
  ESP_LOGD("esp_eg_on", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
}
void config_set(JsonVariant obj)
{
  if (obj.containsKey("mcu_base"))
  {
    JsonArray json_base = obj["mcu_base"].as<JsonArray>();
    config.mcu_base = std::make_tuple(json_base[0].as<String>(), json_base[1].as<String>(), json_base[2].as<String>(), json_base[3].as<String>());
  }
  if (obj.containsKey("mcu_serial"))
  {
    JsonArray json_serial = obj["mcu_serial"].as<JsonArray>();
    config.mcu_serial = std::make_tuple(json_serial[0].as<String>(), json_serial[1].as<int>(), json_serial[2].as<String>());
  }
  if (obj.containsKey("mcu_net"))
  {
    JsonArray json_net = obj["mcu_net"].as<JsonArray>();
    JsonArray json_netap = json_net[1].as<JsonArray>();
    JsonArray json_netsta = json_net[2].as<JsonArray>();
    MyNet::ap_t mcu_net_ap = std::make_tuple(json_netap[0].as<String>());
    MyNet::sta_t mcu_net_sta = std::make_tuple(json_netsta[0].as<String>(), json_netsta[1].as<String>());
    config.mcu_net = std::make_tuple(json_net[0].as<String>(), mcu_net_ap, mcu_net_sta);
  }
  if (obj.containsKey("mcu_dz003"))
  {
    JsonArray json_dz003 = obj["mcu_dz003"].as<JsonArray>();
    config.mcu_dz003 = std::make_tuple(json_dz003[0].as<int>(), json_dz003[1].as<int>(), json_dz003[2].as<int>(), json_dz003[3].as<int>(), json_dz003[4].as<String>());
  }
  if (obj.containsKey("mcu_ybl"))
  {
    a7129namespace::ybl::datas_t ybldatas;
    JsonArray json_ybl = obj["mcu_ybl"].as<JsonArray>();
    JsonObject json_ybldata = json_ybl[1].as<JsonObject>();
    for (const auto& kvp : json_ybldata)
    {
      a7129namespace::ybl::id_t id = kvp.value()["id"].as<a7129namespace::ybl::id_t>();
      a7129namespace::ybl::idInfo_t idinfo = {
          .id = id,
          .type = kvp.value()["type"].as<a7129namespace::ybl::type_t>(),
          .state = kvp.value()["state"].as<a7129namespace::ybl::state_t>() };
      ybldatas.emplace(id, idinfo);
    }
    config.mcu_ybl = std::make_tuple(json_ybl[0].as<String>(), ybldatas, json_ybl[2].as<int>(), json_ybl[3].as<int>());
  }
  if (obj.containsKey("mcu_webPageServer"))
  {
    JsonArray json_webPageServer = obj["mcu_webPageServer"].as<JsonArray>();
    config.mcu_webPageServer = std::make_tuple(json_webPageServer[0].as<String>());
  }
  if (obj.containsKey("mcu_esServer"))
  {
    JsonArray json_esServer = obj["mcu_esServer"].as<JsonArray>();
    config.mcu_esServer = std::make_tuple(json_esServer[0].as<String>());
  }
  if (obj.containsKey("mcu_wsServer"))
  {
    JsonArray json_wsServer = obj["mcu_wsServer"].as<JsonArray>();
    config.mcu_wsServer = std::make_tuple(json_wsServer[0].as<String>(), json_wsServer[1].as<String>());
  }
}
void config_get(JsonVariant obj)
{
  JsonArray json_base = obj.createNestedArray("mcu_base");
  json_base.add(std::get<0>(config.mcu_base));
  json_base.add(std::get<1>(config.mcu_base));
  json_base.add(std::get<2>(config.mcu_base));
  json_base.add(std::get<3>(config.mcu_base));
  JsonArray json_serial = obj.createNestedArray("mcu_serial");
  json_serial.add(std::get<0>(config.mcu_serial));
  json_serial.add(std::get<1>(config.mcu_serial));
  json_serial.add(std::get<2>(config.mcu_serial));
  JsonArray json_net = obj.createNestedArray("mcu_net");
  json_net.add(std::get<0>(config.mcu_net));
  JsonArray json_net_ap = json_net.createNestedArray();
  MyNet::ap_t mcu_net_ap = std::get<1>(config.mcu_net);
  json_net_ap.add(std::get<0>(mcu_net_ap));
  JsonArray json_net_sta = json_net.createNestedArray();
  MyNet::sta_t mcu_net_sta = std::get<2>(config.mcu_net);
  json_net_sta.add(std::get<0>(mcu_net_sta));
  json_net_sta.add(std::get<1>(mcu_net_sta));
  JsonArray json_dz003 = obj.createNestedArray("mcu_dz003");
  json_dz003.add(std::get<0>(config.mcu_dz003));
  json_dz003.add(std::get<1>(config.mcu_dz003));
  json_dz003.add(std::get<2>(config.mcu_dz003));
  json_dz003.add(std::get<3>(config.mcu_dz003));
  json_dz003.add(std::get<4>(config.mcu_dz003));
  JsonArray json_ybl = obj.createNestedArray("mcu_ybl");
  json_ybl.add(std::get<0>(config.mcu_ybl));
  JsonObject json_ybldatas = json_ybl.createNestedObject();
  for (const auto& pair : std::get<1>(config.mcu_ybl))
  {
    JsonObject json_yblidinfo = json_ybldatas.createNestedObject(String(pair.second.id));
    json_yblidinfo["id"] = pair.second.id;
    json_yblidinfo["type"] = pair.second.type;
    json_yblidinfo["state"] = pair.second.state;
  }
  json_ybl.add(std::get<2>(config.mcu_ybl));
  json_ybl.add(std::get<3>(config.mcu_ybl));
  JsonArray json_webPageServer = obj.createNestedArray("mcu_webPageServer");
  json_webPageServer.add(std::get<0>(config.mcu_webPageServer));
  JsonArray json_esServer = obj.createNestedArray("mcu_esServer");
  json_esServer.add(std::get<0>(config.mcu_esServer));
  JsonArray json_wsServer = obj.createNestedArray("mcu_wsServer");
  json_wsServer.add(std::get<0>(config.mcu_wsServer));
  json_wsServer.add(std::get<1>(config.mcu_wsServer));
}
void reqTask(void* nullparam)
{
  myStruct_t myStruct;
  xEventGroupSetBits(state.eg_Handle, EGBIG_REQ);
  ESP_LOGV("ETBIG", "EGBIG_REQ");
  for (;;)
  {
    if (xQueueReceive(state.reqQueueHandle, &myStruct, portMAX_DELAY) == pdPASS)
    {
      if (myStruct.sendTo_name == "mcu_esServer" && state.mcu_webServer->esObj == nullptr) {
        concatAny(myStruct.str, "[\"state.mcu_webServer->esObj==nullptr\",\"%s\"]", myStruct.str.c_str());
        myStruct.sendTo_name = std::get<3>(config.mcu_base);
      }
      else if (myStruct.sendTo_name == "mcu_wsServer" && state.mcu_webServer->wsObj == nullptr) {
        concatAny(myStruct.str, "[\"state.mcu_webServer->wsObj==nullptr\",\"%s\"]", myStruct.str.c_str());
        myStruct.sendTo_name = std::get<3>(config.mcu_base);
      }

      if (myStruct.sendTo_name == "mcu_esServer" && state.mcu_webServer->esObj != nullptr) {
        state.mcu_webServer->esObj->send(myStruct.str.c_str());
      }
      else if (myStruct.sendTo_name == "mcu_wsServer" && state.mcu_webServer->esObj != nullptr) {
        state.mcu_webServer->wsObj->textAll(myStruct.str);
      }
      else
      {
        //concatAny(myStruct.str, "[\"myStruct.sendTo_name error\",\"%s\"]", myStruct.str.c_str());
        state.mcu_serial->println(myStruct.str);
      }
      vTaskDelay(50);
    }
    else
    {
      ESP_LOGD("reqTask", "xQueueReceive != pdPASS");
    }
  }
}
void resTask(void* nullparam)
{
  // uint32_t ptr;
  myStruct_t resStruct;
  xEventGroupSetBits(state.eg_Handle, EGBIG_RES);
  ESP_LOGV("ETBIG", "EGBIG_RES");
  for (;;)
  {
    // if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    // {
    //  myStruct = *(structTypenamespace::myJsonArray_t *)ptr;
    if (xQueueReceive(state.resQueueHandle, &resStruct, portMAX_DELAY) == pdPASS)
    {
      DynamicJsonDocument resdoc(3000);
      DeserializationError error = deserializeJson(resdoc, resStruct.str);
      myStruct_t reqStruct;
      if (error)
      {
        concatAny(reqStruct.str, "[\"resTask error\",\"%s\",\"%s\"]", error.c_str(), resStruct.str.c_str());
        if (xQueueSend(state.reqQueueHandle, &reqStruct, 0) != pdPASS)
        {
          ESP_LOGD("resTask", "reqQueueHandle is full");
        }
      }
      else
      {
        if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
        {
          JsonArray arr = resdoc.as<JsonArray>();
          String api = arr[0].as<String>();
          if (api == "init_get")
          {
            arr.clear();
            arr.add("set");
            JsonObject obj = arr.createNestedObject();
            config_get(obj);
            JsonArray mcu_state = obj.createNestedArray("mcu_state");
            uint32_t ulBits = xEventGroupGetBits(state.eg_Handle); // 获取 Event Group 变量当前值
            mcu_state.add(state.macId);
            JsonArray egBit = mcu_state.createNestedArray();
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
            mcu_state.add(ETH.localIP().toString());
            mcu_state.add(WiFi.localIP().toString());
            mcu_state.add(state.taskindex);
            // JsonObject i18n = obj.createNestedObject("i18n");
            // state.i18n->readFile(i18n);
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
            bool success = state.configFs->writeFile(obj);
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
            bool success = state.configFs->readFile(obj);
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
          else if (api.indexOf("mcu_dz003") > -1)
          {
            dz003namespace::obj->res(arr);
          }
          else if (api.indexOf("mcu_ybl") > -1)
          {
            a7129namespace::ybl::res(arr);
          }
          else
          {
            arr[0].set("mcu pass");
            arr[1].set(api);
          }
          xSemaphoreGive(state.configLock);
          reqStruct.sendTo_name = resStruct.sendTo_name;
          JsonArray mcu_base = arr.createNestedArray();
          mcu_base.add(std::get<0>(config.mcu_base));
          mcu_base.add(std::get<1>(config.mcu_base));
          mcu_base.add(std::get<2>(config.mcu_base));
          serializeJson(arr, reqStruct.str);
          // Serial.println(resdoc.overflowed());
          if (xQueueSend(state.reqQueueHandle, &reqStruct, 0) != pdPASS)
          {
            ESP_LOGE("resJsonArray", "reqQueueHandle is full");
          }
        }
        else
        {
          ESP_LOGE("resJsonArray", "xSemaphoreTake !=pdFALSE");
        }
      }
    }
    else
    {
      ESP_LOGD("resTask", "resTask xQueueReceive != pdPASS");
    }
  }
}
void setup()
{
  Serial.begin(115200);
  // noInterrupts(); // 关闭全局所有中断
  state.macId = String(ESP.getEfuseMac());
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.mcu_serial = &Serial;
  state.reqQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.resQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.configFs = new MyFs("/config.json");
  state.i18nFs = new MyFs("/i18n.json");
  // state.configFs->listFilePrint("/", 5);
  DynamicJsonDocument doc(2000);
  JsonVariant obj = doc.as<JsonVariant>();
  state.configFs->readFile(obj); // 与下行代码交换位置，会不正常
  config_set(obj);
  xEventGroupSetBits(state.eg_Handle, EGBIG_CONFIGJSON);
  /*
  obj["t"] = "config.json";
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("ETBIG", "CONFIGJSON");
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());
  doc.clear();
  config_get(obj);
  obj["t"] = "config_get";
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());
  doc.clear();
  state.i18nFs->readFile(obj);
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());
  */
  xEventGroupWaitBits(state.eg_Handle, EGBIG_CONFIGJSON, pdFALSE, pdTRUE, portMAX_DELAY);
  xTaskCreate(resTask, "resTask", 1024 * 8, NULL, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_RES, pdFALSE, pdTRUE, portMAX_DELAY);
  xTaskCreate(reqTask, "reqTask", 1024 * 4, NULL, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_REQ, pdFALSE, pdTRUE, portMAX_DELAY);

  ESP_ERROR_CHECK(esp_task_wdt_init(20000, false)); // 初始化看门狗
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  ESP_ERROR_CHECK(esp_task_wdt_status(NULL));
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
  esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, esp_eg_on, (void*)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void*)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void*)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void*)__func__));
  ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void*)__func__));

  state.mcu_serial->begin(std::get<1>(config.mcu_serial));
  state.mcu_serial->onReceive([]()
    {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      myStruct_t myStruct = {
        .sendTo_name = std::get<0>(config.mcu_serial),
        .str = state.mcu_serial->readStringUntil('\n') };
      if (xQueueSendFromISR(state.resQueueHandle, &myStruct, &xHigherPriorityTaskWoken) != pdPASS)
        ESP_LOGE("mcu_serial_callback", "Queue is full");
      if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); });

  state.mcu_net = new MyNet(config.mcu_net);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_NET, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_webServer = new MyWebServer(80);
  state.mcu_webServer->webPageServerInit(config.mcu_webPageServer);
  state.mcu_webServer->wsServerInit(config.mcu_wsServer, [](const String& str) -> void
    {
      myStruct_t obj = {
        .sendTo_name = std::get<1>(config.mcu_wsServer),
        .str = str };
      if (xQueueSend(state.resQueueHandle, &obj, 50) != pdPASS)
        ESP_LOGV("mcu_dz003CallBack", "Queue is full"); });
  state.mcu_webServer->esServerInit(config.mcu_esServer);
  state.mcu_webServer->arduinoOtaInit([](const String& message) -> void
    { ESP_LOGV("debug", "%s", message); });

  state.mcu_yblTaskParam = new a7129namespace::ybl::taskParam_t{
      .config = config.mcu_ybl,
      .startCallBack = []()
      { xEventGroupSetBits(state.eg_Handle, EGBIG_YBL);
       ESP_LOGV("ETBIG", "EGBIG_YBL"); },
      .tickCallBack = []()
      {
        myStruct_t obj = {
          .sendTo_name = std::get<0>(config.mcu_ybl),
          .str = "[\"mcu_ybl\"]"};
        if (xQueueSend(state.resQueueHandle, &obj, 50) != pdPASS)
           ESP_LOGV("mcu_yblCallBack", "Queue is full"); } };
  xTaskCreate(a7129namespace::ybl::mainTask, "mcu_yblTask", 1024 * 6, (void*)state.mcu_yblTaskParam, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_YBL, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_dz003TaskParam = new dz003namespace::mainTaskParam_t{
      .config = config.mcu_dz003,
      .startCallBack = []()
      { xEventGroupSetBits(state.eg_Handle, EGBIG_DZ003);
       ESP_LOGV("ETBIG", "EGBIG_DZ003"); },
      .tickCallBack = []()
      {
        myStruct_t obj = {
          .sendTo_name = std::get<4>(config.mcu_dz003),
          .str = "[\"mcu_dz003\"]"};
      if (xQueueSend(state.resQueueHandle, &obj, 50) != pdPASS)
        ESP_LOGV("mcu_dz003CallBack", "Queue is full"); } };
  xTaskCreate(dz003namespace::mainTask, "mcu_dz003Task", 1024 * 6, (void*)state.mcu_dz003TaskParam, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_DZ003, pdFALSE, pdTRUE, portMAX_DELAY);

  vTaskDelete(NULL);
}
void loop(void)
{
}