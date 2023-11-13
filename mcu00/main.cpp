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
#define EGBIG_DZ003 (1 << 4)
struct config_t
{
  std::tuple<String, String, String, String> mcu_base;
  std::tuple<String, int> mcu_serial;
  MyNet::config_t mcu_net;
  dz003namespace::config_t mcu_dz003;
  a7129namespace::ybl::config_t mcu_ybl;
};
config_t config;
struct state_t
{
  String macId;
  int taskindex;
  String locIp;
  unsigned int testindex;
  EventGroupHandle_t eg_Handle;
  SemaphoreHandle_t configLock;
  QueueHandle_t reqQueueHandle;
  QueueHandle_t resQueueHandle;
  MyFs *configFs;
  MyFs *i18n;
  MyNet *mcu_net;
  HardwareSerial *mcu_serial;
  dz003namespace::mainTaskParam_t *mcu_dz003TaskParam;
  a7129namespace::ybl::taskParam_t *mcu_yblTaskParam;
};
state_t state;
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
void config_set(JsonVariant obj)
{
  if (obj.containsKey("mcu_base"))
  {
    JsonArray mcu_base = obj["mcu_base"].as<JsonArray>();
    config.mcu_base = std::make_tuple(mcu_base[0].as<String>(), mcu_base[1].as<String>(), mcu_base[2].as<String>(), mcu_base[3].as<String>());
  }
  if (obj.containsKey("mcu_serial"))
  {
    JsonArray mcu_serial = obj["mcu_serial"].as<JsonArray>();
    config.mcu_serial = std::make_tuple(mcu_serial[0].as<String>(), mcu_serial[1].as<int>());
  }
  if (obj.containsKey("mcu_net"))
  {
    JsonArray mcu_net = obj["mcu_net"].as<JsonArray>();
    std::get<0>(config.mcu_net) = mcu_net[0].as<String>();
    JsonArray mcu_net_ap = mcu_net[1].as<JsonArray>();
    std::get<1>(config.mcu_net) = std::make_tuple(mcu_net_ap[0].as<String>());
    JsonArray mcu_net_sta = mcu_net[2].as<JsonArray>();
    std::get<2>(config.mcu_net) = std::make_tuple(mcu_net_sta[0].as<String>(), mcu_net_sta[1].as<String>());
  }
  if (obj.containsKey("mcu_dz003"))
  {
    JsonArray mcu_dz003 = obj["mcu_dz003"].as<JsonArray>();
    config.mcu_dz003 = std::make_tuple(mcu_dz003[0].as<int>(), mcu_dz003[1].as<int>(), mcu_dz003[2].as<int>(), mcu_dz003[3].as<int>(), mcu_dz003[4].as<String>());
  }
  if (obj.containsKey("mcu_ybl"))
  {
    JsonArray mcu_ybl = obj["mcu_ybl"].as<JsonArray>();
    std::get<0>(config.mcu_ybl) = mcu_ybl[0].as<String>();
    // JsonArray mcu_ybluseIds = mcu_ybl[1].as<JsonArray>();
    // for (int i = 0; i < mcu_ybluseIds.size(); ++i)
    // {
    //   std::get<1>(config.mcu_ybl)[i] = mcu_ybluseIds[i].as<a7129namespace::id_t>();
    // }
  }
}
void config_get(JsonVariant obj)
{
  JsonArray mcu_base = obj.createNestedArray("mcu_base");
  mcu_base.add(std::get<0>(config.mcu_base));
  mcu_base.add(std::get<1>(config.mcu_base));
  mcu_base.add(std::get<2>(config.mcu_base));
  mcu_base.add(std::get<3>(config.mcu_base));
  JsonArray mcu_serial = obj.createNestedArray("mcu_serial");
  mcu_serial.add(std::get<0>(config.mcu_serial));
  mcu_serial.add(std::get<1>(config.mcu_serial));
  JsonArray mcu_net = obj.createNestedArray("mcu_net");
  mcu_net.add(std::get<0>(config.mcu_net));
  JsonArray muc_net_ap = mcu_net.createNestedArray();
  MyNet::ap_t ap = std::get<1>(config.mcu_net);
  muc_net_ap.add(std::get<0>(ap));
  JsonArray muc_net_sta = mcu_net.createNestedArray();
  MyNet::sta_t sta = std::get<2>(config.mcu_net);
  muc_net_sta.add(std::get<0>(sta));
  muc_net_sta.add(std::get<1>(sta));
  JsonArray mcu_dz003 = obj.createNestedArray("mcu_dz003");
  mcu_dz003.add(std::get<0>(config.mcu_dz003));
  mcu_dz003.add(std::get<1>(config.mcu_dz003));
  mcu_dz003.add(std::get<2>(config.mcu_dz003));
  mcu_dz003.add(std::get<3>(config.mcu_dz003));
  mcu_dz003.add(std::get<4>(config.mcu_dz003));
  JsonArray mcu_ybl = obj.createNestedArray("mcu_ybl");
  mcu_ybl.add(std::get<0>(config.mcu_ybl));
  // JsonArray mcu_ybluseIds = mcu_ybl.createNestedArray();
  // for (const auto &element : std::get<1>(config.mcu_ybl))
  // {
  //   if (element)
  //     mcu_ybluseIds.add(element);
  // }
}
void reqTask(void *nullparam)
{
  myStruct_t myStruct;
  ESP_LOGV("reqTask", "%d", ESP.getFreeHeap());
  for (;;)
  {
    if (xQueueReceive(state.reqQueueHandle, &myStruct, portMAX_DELAY) == pdPASS)
    {
      if (myStruct.sendTo_name == "mcu_serial")
      {
        state.mcu_serial->println(myStruct.str);
      }
      else
      {
        myStruct.str = String("[\"sendTo_name undefind\",\"") + myStruct.str + String("\"]");
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
void resTask(void *nullparam)
{
  // uint32_t ptr;
  myStruct_t resStruct;
  ESP_LOGV("resTask", "%d", ESP.getFreeHeap());
  for (;;)
  {
    // if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
    // {
    //  myStruct = *(structTypenamespace::myJsonArray_t *)ptr;
    if (xQueueReceive(state.resQueueHandle, &resStruct, portMAX_DELAY) == pdPASS)
    {
      // StaticJsonDocument<2000> resdoc;
      DynamicJsonDocument resdoc(3000);
      DeserializationError error = deserializeJson(resdoc, resStruct.str);
      if (error)
      {
        resStruct.sendTo_name = std::get<3>(config.mcu_base);
        resStruct.str = "[\"resTask error\",\"" + String(error.c_str()) + "\",\"" + resStruct.str + "\"]";
        if (xQueueSend(state.reqQueueHandle, &resStruct, 0) != pdPASS)
        {
          ESP_LOGD("resTask", "reqQueueHandle is full");
        }
      }
      else
      {
        JsonArray arr = resdoc.as<JsonArray>();
        String api = arr[0].as<String>();
        if (xSemaphoreTake(state.configLock, portMAX_DELAY) == pdTRUE)
        {
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
            mcu_state.add(state.locIp);
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
            state.mcu_dz003TaskParam->obj.res(arr);
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
          myStruct_t reqStruct;
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
void mcu_yblCallBack(void)
{
  myStruct_t obj = {
      .sendTo_name = std::get<0>(config.mcu_ybl),
      .str = "[\"mcu_ybl\"]"};
  if (xQueueSend(state.resQueueHandle, &obj, 50) != pdPASS)
  {
    ESP_LOGV("mcu_yblCallBack", "Queue is full");
  }
}
void mcu_dz003CallBack(void)
{
  myStruct_t data = {
      .sendTo_name = std::get<4>(config.mcu_dz003),
      .str = "[\"mcu_dz003\"]"};
  if (xQueueSend(state.resQueueHandle, &data, 50) != pdPASS)
  {
    ESP_LOGV("mcu_dz003CallBack", "Queue is full");
  }
}
void mcu_serialCallBack(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  myStruct_t myStruct = {
      .sendTo_name = std::get<0>(config.mcu_serial),
      .str = state.mcu_serial->readStringUntil('\n')};
  if (xQueueSendFromISR(state.resQueueHandle, &myStruct, &xHigherPriorityTaskWoken) != pdPASS)
  {
    ESP_LOGE("mcu_serial_callback", "Queue is full");
  }
  if (xHigherPriorityTaskWoken == pdTRUE)
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}
void setup()
{
  Serial.begin(115200);
  // noInterrupts(); // 关闭全局所有中断
  state.macId = String(ESP.getEfuseMac());
  state.testindex = 0;
  state.eg_Handle = xEventGroupCreate();
  state.configLock = xSemaphoreCreateMutex();
  state.reqQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.resQueueHandle = xQueueCreate(10, sizeof(myStruct_t));
  state.mcu_serial = &Serial;
  // state.configFs->listFilePrint("/", 3);
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
  // state.configFs->listFilePrint("/", 5);
  DynamicJsonDocument doc(2000);
  JsonVariant obj = doc.as<JsonVariant>();
  state.configFs = new MyFs("/config.json");
  state.i18n = new MyFs("/i18n.json");
  state.configFs->readFile(obj); // 与下行代码交换位置，会不正常
  config_set(obj);
  obj["t"] = "config.json";
  obj["testindex"] = state.testindex++;
  serializeJson(obj, *state.mcu_serial);
  xEventGroupSetBits(state.eg_Handle, EGBIG_CONFIG);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());
  doc.clear();
  config_get(obj);
  obj["t"] = "config_get";
  obj["testindex"] = state.testindex++;
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());

  doc.clear();
  state.i18n->readFile(obj);
  obj["testindex"] = state.testindex++;
  serializeJson(obj, *state.mcu_serial);
  ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap());

  xEventGroupWaitBits(state.eg_Handle, EGBIG_CONFIG, pdFALSE, pdTRUE, portMAX_DELAY);
  state.mcu_serial->begin(std::get<1>(config.mcu_serial));
  state.mcu_serial->onReceive(mcu_serialCallBack);

  state.mcu_net = new MyNet(config.mcu_net);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_NET, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_yblTaskParam = new a7129namespace::ybl::taskParam_t{
      .taskindex = &state.taskindex,
      .config = config.mcu_ybl,
      .startCallBack = []()
      { xEventGroupSetBits(state.eg_Handle, EGBIG_YBL); },
      .tickCallBack = mcu_yblCallBack};
  xTaskCreate(a7129namespace::ybl::mainTask, "mcu_yblTask", 1024 * 6, (void *)state.mcu_yblTaskParam, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_YBL, pdFALSE, pdTRUE, portMAX_DELAY);

  state.mcu_dz003TaskParam = new dz003namespace::mainTaskParam_t{
      .config = config.mcu_dz003,
      .obj = dz003namespace::Dz003Class(),
      .startCallBack = []()
      { xEventGroupSetBits(state.eg_Handle, EGBIG_DZ003); },
      .tickCallBack = mcu_dz003CallBack};
  xTaskCreate(dz003namespace::mainTask, "mcu_dz003Task", 1024 * 6, (void *)state.mcu_dz003TaskParam, state.taskindex++, NULL);
  xEventGroupWaitBits(state.eg_Handle, EGBIG_DZ003, pdFALSE, pdTRUE, portMAX_DELAY);

  xTaskCreate(resTask, "resTask", 1024 * 8, NULL, state.taskindex++, NULL);
  xTaskCreate(reqTask, "reqTask", 1024 * 4, NULL, state.taskindex++, NULL);
  // interrupts(); // 打开全局所有中断
  //  vTaskStartScheduler();//
  vTaskDelete(NULL);
}
void loop(void)
{
}