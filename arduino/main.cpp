#include <string>
#include <tuple>
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
// #include <SPIFFS.h>
// #include <IPAddress.h>
// #include <ArduinoJson.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncWebSocket.h>
// #include <structTypenamespace.h>
// #include <serialTasknamespace.h>
// #include <dz003namespace.h>
// #include <filenamespace.h>
// #include <netnamespace.h>
#include <a7129namespace.h>
// #define fileOs SPIFFS
#define uartDef Serial
#define EGBIG_FILEOS (1 << 0)
#define EGBIG_CONFIG (2 << 0)
#define EGBIG_NET (3 << 0)
// AsyncWebServer *webServer;
// AsyncWebSocket *wsServer;
typedef struct
{
  String packageName;
  String mcuId;
  String locIp;
  int taskindex;
} state_t;
state_t state;
typedef struct
{
  // netnamespace::initParam_t server_net;
  a7129namespace::config_t server_ybl;
  // dz003namespace::config_t server_dz003;
  std::tuple<std::string> server_html;
  std::tuple<std::string> client_html;
} config_t;
config_t config;
EventGroupHandle_t eg_Handle = xEventGroupCreate();
String GLOBALFILEPATH = "/config.json";
// TaskHandle_t stdStringTaskHandle, jsonArrayTaskHandle, server_serialTaskHandle, server_dz003TaskHandle;
// SemaphoreHandle_t globalConfigLock;
//  String mcuId_get(void)
//  {
//    return String(ESP.getEfuseMac(), HEX);
//  }
//  void config_set(JsonObject &obj)
//  {
//    JsonArray arr;
//    if (obj.containsKey("server_dz003"))
//    {
//      arr = obj["server_dz003"].as<JsonArray>();
//      config.server_dz003 = std::make_tuple(
//          arr[0].as<int>(),
//          arr[1].as<int>(),
//          arr[2].as<int>(),
//          arr[3].as<int>(),
//          arr[4].as<std::string>());
//    }
//  }
//  jsonObject &config_get(void)
//  {
//  }
//  void config_set_toFile(void)
//  {
//    File dataFile = fileOs.open(GLOBALFILEPATH, "w");
//    DeserializationError error = serializeJson(config_get(), dataFile);
//    dataFile.close();
//    if (error)
//    {
//      ESP_LOGV("DEBUE", "%s", error.c_str());
//    }
//  }
//  void config_set_fromFile(void)
//  {
//    if (!fileOs.exists(GLOBALFILEPATH))
//    {
//      ESP_LOGV("DEBUE", " !fileOs.exists(GLOBALFILEPATH)");
//      return;
//    }
//    DynamicJsonDocument c(3000);
//    File dataFile = fileOs.open(GLOBALFILEPATH);
//    DeserializationError error = deserializeJson(c, dataFile);
//    dataFile.close();
//    if (error)
//    {
//      ESP_LOGV("DEBUE", "Error deserializing JSON:%s", error.c_str());
//    }
//    else
//    {
//      JsonObject json = c.to<JsonObject>();
//      config_set(json);
//      xEventGroupSetBits(eg_Handle, EGBIG_CONFIG);
//      // serializeJson(c, uartDef);
//      //   serializeJsonPretty(globalConfig, uartDef);
//    }
//  }
// void esp_eg_on(void *registEr, esp_event_base_t postEr, int32_t eventId, void *eventData)
// {
//   // EventBits_t bits = xEventGroupWaitBits(eg_Handle, EGBIG_NET | EGBIG_NET | dz003_bit, pdFALSE, pdTRUE, portMAX_DELAY);
//   // xEventGroupClearBits(eg_Handle, EGBIG_NET | BIT_2);
//   char *er = (char *)registEr;
//   int use = 0;
//   if (postEr == IP_EVENT && eventId == 4)
//   {
//     // ESP_LOGV("DEBUG", "ETH.localIP:%s", ETH.localIP().toString().c_str());
//     // state.locIp = String(ETH.localIP());
//     xEventGroupSetBits(eg_Handle, EGBIG_NET);
//     use = 1;
//   }
//   else if (postEr == IP_EVENT && eventId == 0)
//   {
//     ESP_LOGV("DEBUG", "WiFi.localIP:%s", WiFi.localIP().toString().c_str());
//     // state.locIp = String(WiFi.localIP());
//     xEventGroupSetBits(eg_Handle, EGBIG_NET);
//     use = 1;
//   }
//   // char *data = eventData ? ((char *)eventData) : ((char *)"");
//   // ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d,eventData:%s", er, use, postEr, eventId, (char *)eventData);
//   ESP_LOGV("DEBUG", "registEr:%s,use:%d,postEr:%s, eventId:%d", er, use, postEr, eventId);
// }
// void server_esp(void)
// {
//   ESP_LOGV("getFreeHeap", "%d", ESP.getFreeHeap(), ESP_OK);
//   ESP_ERROR_CHECK(esp_task_wdt_init(20000, false)); // 初始化看门狗
//   ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
//   ESP_ERROR_CHECK(esp_task_wdt_status(NULL));
//   esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
//   esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
//   ESP_ERROR_CHECK(esp_event_loop_create_default());
//   ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
//   ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
//   ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
//   ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, esp_eg_on, (void *)__func__));
// }
//  void sendEr(structTypenamespace::notifyString_t *strObj)
//  {
//    if (strObj->sendTo_name == "server_serial")
//    {
//      serialTasknamespace::send(strObj->msg);
//    }
//    else
//    {
//      strObj->msg = "[\"sendTo_name undefind\"]";
//      serialTasknamespace::send(strObj->msg);
//    }
//  }
//  void jsonArrayParse(structTypenamespace::notifyJsonArray_t &arrObj)
//  {
//    if (xSemaphoreTake(globalConfigLock, portMAX_DELAY) == pdTRUE)
//    {
//      JsonArray doc = arrObj.msg;
//      String api = doc[0].as<String>();
//      String msg;
//      if (api == "config_set")
//      {
//        for (auto kv : doc[1].as<JsonObject>())
//        {
//          if (globalConfig.containsKey(kv.key()))
//          {
//            globalConfig[kv.key()].set(kv.value());
//          };
//        };
//      }
//      else if (api == "config_get")
//      {
//        doc[0].set("config_set");
//        doc[1].set(globalConfig);
//        serializeJson(doc, msg);
//      }
//      else if (api == "config_toFile")
//      {
//        File file = fileOs.open(GLOBALFILEPATH, "w");
//        serializeJson(globalConfig, file);
//        file.close();
//        msg = "[\"config_toFile\"]";
//      }
//      else if (api == "config_fromFile")
//      {
//        globalConfig_fromFile();
//        doc[0].set("config_set");
//        doc[1].set(globalConfig);
//        serializeJson(doc, msg);
//      }
//      else if (api == "restart")
//      {
//        msg = "[\"api restart\"]";
//        ESP.restart(); // 重启复位esp32
//      }
//      else if (api == "state_get")
//      {
//        uint32_t ulBits = xEventGroupGetBits(eg_Handle); // 获取 Event Group 变量当前值
//        doc[0].set("state_set");
//        DynamicJsonDocument info(100);
//        JsonObject egBit = info.createNestedObject("egBit");
//        for (int i = sizeof(ulBits) * 8 - 1; i >= 0; i--)
//        { // 循环输出每个二进制位
//          uint32_t mask = 1 << i;
//          if (ulBits & mask)
//          {
//            egBit[String(i)] = true;
//          }
//          else
//          {
//            egBit[String(i)] = false;
//          }
//        }
//        info["locIp"] = state.locIp;
//        doc[1].set(info);
//        serializeJson(doc, msg);
//      }
//      else if (api == "dz003State")
//      {
//        msg = dz003Tasknamespace::state();
//      }
//      else if (api == "dz003.fa_set")
//      {
//        dz003Tasknamespace::fa_set(doc[1].as<bool>());
//        msg = dz003Tasknamespace::state();
//      }
//      else if (api == "dz003.frequency_set")
//      {
//        dz003Tasknamespace::frequency_set(doc[1].as<bool>());
//        msg = dz003Tasknamespace::state();
//      }
//      else if (api == "dz003.laba_set")
//      {
//        dz003Tasknamespace::laba_set(doc[1].as<bool>());
//        msg = dz003Tasknamespace::state();
//      }
//      else if (api == "dz003.deng_set")
//      {
//        dz003Tasknamespace::deng_set(doc[1].as<bool>());
//        msg = dz003Tasknamespace::state();
//      }
//      else
//      {
//        msg = "[\"mcu pass\",\"" + api + "\"]";
//      }
//      structTypenamespace::notifyString_t strObj = {arrObj->sendTo_name, msg};
//      sendEr(&strObj);
//      xSemaphoreGive(globalConfigLock);
//    }
//  }
//  void stdStringParse(structTypenamespace::notifyString_t &strObj)
//  {
//    DynamicJsonDocument doc(3000);
//    DeserializationError error = deserializeJson(doc, strObj.msg);
//    if (error)
//    {
//      strObj.msg = "[\"json pase error\",\"" + String(error.c_str()) + "\"]";
//      sendEr(strObj);
//    }
//    else
//    {
//      JsonArray arr = doc.as<JsonArray>();
//      structTypenamespace::notifyJsonArray_t arrObj = {strObj.sendTo_name, arr};
//      jsonArrayParse(arrObj);
//    }
//  }
//  void jsonArrayTask(void *nullparam)
//  {
//    uint32_t ptr;
//    structTypenamespace::notifyJsonArray_t *arrObj; //= new structTypenamespace::notifyJsonArray_t();
//    for (;;)
//    {
//      if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
//      {
//        arrObj = (structTypenamespace::notifyJsonArray_t *)ptr;
//        jsonArrayParse(arrObj);
//        ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
//      }
//    }
//  }
//  void stdStringTask(void *nullparam)
//  {
//    uint32_t ptr;
//    structTypenamespace::notifyString_t *strObj; //= new structTypenamespace::notifyString_t();
//    for (;;)
//    {
//      if (xTaskNotifyWait(pdFALSE, ULONG_MAX, (uint32_t *)&ptr, portMAX_DELAY) == pdPASS)
//      {
//        strObj = (structTypenamespace::notifyString_t *)ptr;
//        stdStringParse(strObj);
//        ESP_LOGV("DEBUE", "%s", strObj->sendTo_name.c_str());
//        ulTaskNotifyValueClear(xTaskGetCurrentTaskHandle(), ptr); // 清除通知值
//      }
//    }
//  }
//  bool fileOs_server()
//  {
//    if (fileOs.begin(true))
//    {
//      xEventGroupSetBits(eg_Handle, EGBIG_CONFIG);
//    }
//  }
//  void server_serial(void)
//  {
//    Serial.onReceive([]()
//                     {
//       structTypenamespace::notifyString_t strObj = {
//        globalConfig["server"]["serial"][0].as<const char *>(),
//        Serial.readStringUntil('\n')
//        };
//    stdStringParse(&strObj); });
//  }
//  void server_ws(void)
//  {
//    wsServer->onEvent([=](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
//                      {
//        String wsOn_db = "";
//        if (type == WS_EVT_CONNECT)
//        {
//          wsServer->printfAll("[\"Hello Client :\", %u]", client->id());
//        }
//        else if (type == WS_EVT_DATA)
//        {
//          AwsFrameInfo *info = (AwsFrameInfo *)arg;
//          if (info->final && info->index == 0 && info->len == len)
//          {
//            if (info->opcode == WS_TEXT)
//            {
//              for (size_t i = 0; i < info->len; i++)
//              {
//                wsOn_db += (char)data[i];
//              }
//            }
//            else
//            {
//              char buff[3];
//              for (size_t i = 0; i < info->len; i++)
//              {
//                sprintf(buff, "%02x ", (uint8_t)data[i]);
//                wsOn_db += buff;
//              }
//            }
//          }
//          else
//          {
//            if (info->opcode == WS_TEXT)
//            {
//              for (size_t i = 0; i < len; i++)
//              {
//                wsOn_db += (char)data[i];
//              }
//            }
//            else
//            {
//              char buff[3];
//              for (size_t i = 0; i < len; i++)
//              {
//                sprintf(buff, "%02x ", (uint8_t)data[i]);
//                wsOn_db += buff;
//              }
//            }
//          }

//       } });
// }
// void server_ota(void)
// {
// }
// void server_html(void)
// {
//   //"http://39.97.216.195:8083/index.html?wsIp="
//   webServer->on("*", HTTP_GET, [](AsyncWebServerRequest *request)
//                 {
//       AsyncResponseStream *response = request->beginResponseStream("text/html");
//       response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
//       response->printf("<p>request->host:%s%s</p>", request->host().c_str(), request->url().c_str());
//       response->printf("<p>WiFi.softAPIP:%s</p>", WiFi.softAPIP().toString().c_str());
//       response->printf("<p>WiFi.localIP:%s</p>", WiFi.localIP().toString().c_str());
//       response->printf("<button onclick=\"window.location.href='http://%s/index.html?wsIp='\">net</button>",state.locIp.c_str(),WiFi.localIP().toString().c_str());
//       response->printf("<button onclick=\"window.location.href='http://%s/index.html?wsIp='\">互联网ws通信</button>",state.locIp.c_str(),WiFi.localIP().toString().c_str());
//       response->printf("<button onclick=\"window.location.href='http://%s/index.html?wsIp='\">互联网serial通信</button>",state.locIp.c_str(),WiFi.localIP().toString().c_str());
//       response->print("</body></html>");
//       request->send(response); });
// }
// void server_static(void)
// {
//   webServer->serveStatic("/", fileOs, "/").setCacheControl("max-age=600");
//   webServer->addHandler(wsServer);
//   DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
// }
// void server_http(void)
// {
// }
// void server_events(void)
// {
// }

void setup(void)
{
  uartDef.begin(115200);
  // globalConfigLock = xSemaphoreCreateMutex();
  // fileOs_server();
  // xEventGroupWaitBits(eg_Handle, EGBIG_FILEOS, pdTRUE, pdTRUE, portMAX_DELAY);
  //server_esp();
  // config_set_fromFile();
  // xEventGroupWaitBits(eg_Handle, EGBIG_CONFIG, pdTRUE, pdTRUE, portMAX_DELAY);
  // ESP_LOGV("DEBUE", "%s", "---------------EGBIG_CONFIG  SUCCESS----------------");
  // xTaskCreate(stdStringTask, "stdStringTask", 1024 * 10, NULL, state.taskindex++, &stdStringTaskHandle);
  // xTaskCreate(jsonArrayTask, "jsonArrayTask", 1024 * 10, NULL, state.taskindex++, &jsonArrayTaskHandle);
  // server_serial();
  // netnamespace::server_net(config.server_net);
  // xEventGroupWaitBits(eg_Handle, EGBIG_NET, pdTRUE, pdTRUE, portMAX_DELAY);
  // ESP_LOGV("DEBUE", "%s", "---------------EGBIG_NET SUCCESS------------------\n");

  // dz003namespace::taskParam_t *server_dz003Param;
  // server_dz003Param->sendTo_taskHandle = stdStringTaskHandle;
  // server_dz003Param->config = config.server_dz003;
  // xTaskCreate(dz003namespace::resTask, "server_dz003_Task", 1024 * 6, (void *)server_dz003Param, state.taskindex++, &server_dz003TaskHandle);

  // a7129namespace::taskParam_t *server_yblParam;
  // server_yblParam->sendTo_taskHandle = stdStringTaskHandle;
  // server_yblParam->config = config.server_ybl;
  xTaskCreate(a7129namespace::yblResTask, "ybResTask", 1024 * 4, (void *)&config.server_ybl, state.taskindex++, NULL);
 // xTaskCreate(a7129namespace::yblResTask, "ybResTask", 1024 * 7, NULL, 5, NULL);
  // std::unordered_map 对象格式的json字符串
  vTaskStartScheduler();
  // vTaskDelete(NULL);
}
void loop(void)
{
  // wsServer->cleanupClients(3);
  vTaskDelay(10000);
}

// 等待锁
//  if (xSemaphoreTake(globalConfigLock, portMAX_DELAY) == pdTRUE)
//  {
//    // 对共享数据进行操作
//    // 释放数据锁
//    xSemaphoreGive(globalConfigLock);
//  }