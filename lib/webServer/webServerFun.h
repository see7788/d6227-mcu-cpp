// #ifndef webServerFun_h
// #define webServerFun_h
// #include <Arduino.h>
// #include <esp_log.h>
// #include <SPIFFS.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncWebSocket.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <freertos/event_groups.h>
// namespace webServerFun
// {
//   AsyncWebServer webServer(80);
//   AsyncWebSocket wsServer("/ws");
//   void send(String str)
//   {
//     wsServer.textAll(str);
//   }
//   TaskHandle_t wsOnTask_Handle;
//   typedef struct
//   {
//     int *taskindex;
//     JsonObject config;
//     void (*onErTaskFun)(void *);
//     void (*sendFun)(String);
//   } taskPtr_t;
//   String wsOn_db;
//   void taskA(void *onTaskPtr)
//   {
//     taskPtr_t param = *(taskPtr_t *)onTaskPtr;
//     JsonObject c = param.config;
//     if (c["init"].as<bool>() == false)
//     {
//       ESP_LOGE("debug", "webServer.init==false");
//       vTaskDelete(NULL);
//     }
//     else
//     {
//       if (pdPASS != xTaskCreate(param.onErTaskFun, "onWsErTask", 1024 * 6, (void *)param.sendFun, *(param.taskindex)++, &wsOnTask_Handle))
//       {
//         ESP_LOGE("debug", "onWsErTask error");
//       }
//       webServer.serveStatic("/", SPIFFS, "/").setCacheControl("max-age=600");
//       webServer.addHandler(&wsServer);
//       DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
//       webServer.begin();
//       ESP_LOGV("start", "");
//       webServer.on("*", HTTP_GET, [c](AsyncWebServerRequest *request)
//                    {
//   AsyncResponseStream *response = request->beginResponseStream("text/html");
//   response->print(F("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>"));
//   response->printf("<p>request->host:%s%s</p>", request->host().c_str(), request->url().c_str());
//   //response->printf("<p>WiFi.softAPIP:%s</p>", WiFi.softAPIP().toString().c_str());
//   response->printf("<p>WiFi.localIP:%s</p>", WiFi.localIP().toString().c_str());
//   response->printf("<button onclick=\"window.location.href='%s%s'\">wifi go</button>",c["internet_indexUrl"].as<String>().c_str(),WiFi.localIP().toString().c_str());
//   response->printf("<p>ETH.localIP:%s</p>", ETH.localIP().toString().c_str());
//   response->printf("<button onclick=\"window.location.href='%s%s'\">ETH go</button>",c["internet_indexUrl"].as<String>().c_str(),ETH.localIP().toString().c_str());
//   response->print("</body></html>");
//   request->send(response); });
//       wsServer.onEvent([=](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
//                        {
//   wsOn_db = "";
//   if (type == WS_EVT_CONNECT)
//   {
//     wsServer.printfAll("[\"Hello Client :\", %u]", client->id());
//   }
//   else if (type == WS_EVT_DATA)
//   {
//     AwsFrameInfo *info = (AwsFrameInfo *)arg;
//     if (info->final && info->index == 0 && info->len == len)
//     {
//       if (info->opcode == WS_TEXT)
//       {
//         for (size_t i = 0; i < info->len; i++)
//         {
//           wsOn_db += (char)data[i];
//         }
//       }
//       else
//       {
//         char buff[3];
//         for (size_t i = 0; i < info->len; i++)
//         {
//           sprintf(buff, "%02x ", (uint8_t)data[i]);
//           wsOn_db += buff;
//         }
//       }
//     }
//     else
//     {
//       if (info->opcode == WS_TEXT)
//       {
//         for (size_t i = 0; i < len; i++)
//         {
//           wsOn_db += (char)data[i];
//         }
//       }
//       else
//       {
//         char buff[3];
//         for (size_t i = 0; i < len; i++)
//         {
//           sprintf(buff, "%02x ", (uint8_t)data[i]);
//           wsOn_db += buff;
//         }
//       }
//     }
//     // ESP_LOGV("--------1111-------", "%s/n",wsOn_db.c_str());
//     xTaskNotify(wsOnTask_Handle, (uint32_t)&wsOn_db, eSetValueWithOverwrite);
//   } });
//       // vTaskDelete(NULL);
//       for (;;)
//       {
//         wsServer.cleanupClients(3);
//         vTaskDelay(5000);
//       }
//     }
//   }
// }
// #endif