#ifndef webServernamespace_h
#define webServernamespace_h
#include <Arduino.h>
#include <esp_log.h>
#include <SPIFFS.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
class MyWebServer
{
public:
  AsyncWebServer *server;
  AsyncWebSocket *ws;
  AsyncEventSource *events;
  typedef std::tuple<int> op_base_t;
  MyWebServer()
  {
    server = new AsyncWebServer(80);
    server->begin();
  };
  typedef std::tuple<String> mcu_ws_op_t;
  void mcu_ws(void)
  {
    ws = new AsyncWebSocket("/");
    ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                {
       String wsOn_db = "";
       if (type == WS_EVT_CONNECT)
       {
         ws->printfAll("[\"Hello Client :\", %u]", client->id());
       }
       else if (type == WS_EVT_DATA)
       {
         AwsFrameInfo *info = (AwsFrameInfo *)arg;
         if (info->final && info->index == 0 && info->len == len)
         {
           if (info->opcode == WS_TEXT)
           {
             for (size_t i = 0; i < info->len; i++)
             {
               wsOn_db += (char)data[i];
             }
           }
           else
           {
             char buff[3];
             for (size_t i = 0; i < info->len; i++)
             {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               wsOn_db += buff;
             }
           }
         }
         else
         {
           if (info->opcode == WS_TEXT)
           {
             for (size_t i = 0; i < len; i++)
             {
               wsOn_db += (char)data[i];
             }
           }
           else
           {
             char buff[3];
             for (size_t i = 0; i < len; i++)
             {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               wsOn_db += buff;
             }
           }
         }
      } });
    server->addHandler(ws);
  };
  typedef std::tuple<String> mcu_events_op_t;
  void mcu_events(void)
  {
    events->onConnect([](AsyncEventSourceClient *client)
                      { client->send("hello!", NULL, millis(), 1000); });
    server->addHandler(events);
  };
  typedef std::tuple<String> mcu_ota_op_t;
  void mcu_ota(void)
  {
    ArduinoOTA.setHostname("esp-async");
    ArduinoOTA.onStart([this]()
                       { events->send("Update Start", "ota"); });
    ArduinoOTA.onEnd([this]()
                     { events->send("Update End", "ota"); });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total)
                          {
    char p[32];
    sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
    events->send(p, "ota"); });
    ArduinoOTA.onError([this](ota_error_t error)
                       {
    if(error == OTA_AUTH_ERROR) events->send("Auth Failed", "ota");
    else if(error == OTA_BEGIN_ERROR) events->send("Begin Failed", "ota");
    else if(error == OTA_CONNECT_ERROR) events->send("Connect Failed", "ota");
    else if(error == OTA_RECEIVE_ERROR) events->send("Recieve Failed", "ota");
    else if(error == OTA_END_ERROR) events->send("End Failed", "ota"); });

    ArduinoOTA.begin();
  };
  typedef std::tuple<String> mcu_html_op_t;
  void mcu_html(void)
  {
    //"http://39.97.216.195:8083/index.html?wsIp="
    server->on("*", HTTP_GET, [](AsyncWebServerRequest *request)
               {
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
      response->printf("<p>request->host:%s%s</p>", request->host().c_str(), request->url().c_str());
      response->printf("<p>WiFi.softAPIP:%s</p>", WiFi.softAPIP().toString().c_str());
      response->printf("<p>WiFi.localIP:%s</p>", WiFi.localIP().toString().c_str());
      // response->printf("<button onclick=\"window.location.href='http://%s/index.html?wsIp='\">互联网ws通信</button>",state.locIp.c_str(),WiFi.localIP().toString().c_str());
      // response->printf("<button onclick=\"window.location.href='http://%s/index.html?wsIp='\">互联网serial通信</button>",state.locIp.c_str(),WiFi.localIP().toString().c_str());
      response->print("</body></html>");
      request->send(response); });
  };
  typedef std::tuple<String> mcu_fs_op_t;
  void mcu_fs(void)
  {
    server->serveStatic("/", SPIFFS, "/").setCacheControl("max-age=600");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  };
  typedef std::tuple<String> mcu_onFileUpload_op_t;
  void mcu_onFileUpload(void)
  {
    server->onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                         {
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len); });
  }
  void mcu_onRequestBody(void)
  {
    server->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                          {
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total); });
  };
  typedef std::tuple<String> mcu_onRequestBody_op_t;
  void mcu_onNotFound(void)
  {
    server->onNotFound([](AsyncWebServerRequest *request)
                       {
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404); });
  };
};
#endif