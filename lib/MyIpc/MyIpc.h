#ifndef MyIpc_h
#define MyIpc_h
#include <Arduino.h>
#include <esp_log.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <AsyncUDP.h>
// #include <Update.h>     //更新固件的核心类
#include <ArduinoOTA.h> //无线方式更新固件，这是基于udp协议的
// #include <HTTPUpdateServer.h>//http方式更新固件，只能基于#include <WebServer.h>进行更新
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

class MyIpc
{
private:
  typedef struct
  {
    String deviceName;
    // 其他连接参数...
  } bleconfig_t;

  typedef struct
  {
    bool mcu_otastart;
  } taskParam_t;
  void rtos(void *ptr)
  {
    taskParam_t *c = (taskParam_t *)ptr;
    if (c->mcu_otastart)
    {
      ArduinoOTA.handle();
    }
  };

public:
  AsyncWebServer *serverObj;
  AsyncWebSocket *wsObj;
  AsyncEventSource *eventsObj;
  AsyncUDP netudp;
  typedef std::tuple<int> base_op_t;
  MyIpc(HardwareSerial *uart)
  {
    serverObj = new AsyncWebServer(80);
    serverObj->begin();
  };
  typedef std::tuple<String> ws_mcu_op_t;
  void ws_mcu(void)
  {
    wsObj = new AsyncWebSocket("/");
    wsObj->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                {
       String wsOn_db = "";
       if (type == WS_EVT_CONNECT)
       {
         wsObj->printfAll("[\"Hello Client :\", %u]", client->id());
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
    serverObj->addHandler(wsObj);
  }
  typedef std::tuple<String> events_mcu_op_t;
  void events_mcu(void)
  {
    eventsObj=new AsyncEventSource("es");
    eventsObj->onConnect([](AsyncEventSourceClient *client)
                      { client->send("hello!", NULL, millis(), 1000); });
    serverObj->addHandler(eventsObj);
  }
  typedef std::tuple<String> ota_mcu_op_t;
  void ota_mcu(void) // 路由，页面样式都是固定的，没找到修改的api
  {
    ArduinoOTA.setHostname("esp-async");
    ArduinoOTA.onStart([this]()
                       { eventsObj->send("Update Start", "ota"); });
    ArduinoOTA.onEnd([this]()
                     { eventsObj->send("Update End", "ota"); });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total)
                          {
    char p[32];
    sprintf(p, "Progress: %u%%\n", (progress/(total/100)));
    eventsObj->send(p, "ota"); });
    ArduinoOTA.onError([this](ota_error_t error)
                       {
    if(error == OTA_AUTH_ERROR) eventsObj->send("Auth Failed", "ota");
    else if(error == OTA_BEGIN_ERROR) eventsObj->send("Begin Failed", "ota");
    else if(error == OTA_CONNECT_ERROR) eventsObj->send("Connect Failed", "ota");
    else if(error == OTA_RECEIVE_ERROR) eventsObj->send("Recieve Failed", "ota");
    else if(error == OTA_END_ERROR) eventsObj->send("End Failed", "ota"); });

    ArduinoOTA.begin();
  }
  // perform the actual update from a given stream
  typedef std::tuple<String> router_index_op_t;
  void router_index_mcu(void)
  {
    //"http://39.97.216.195:8083/index.html?wsIp="
    serverObj->on("*", HTTP_GET, [](AsyncWebServerRequest *request)
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
  }
  typedef std::tuple<String> router_fs_mcu_op_t;
  void router_fs_mcu(void)
  {
    serverObj->serveStatic("/", SPIFFS, "/").setCacheControl("max-age=600");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  }
  typedef std::tuple<String> onFileUpload_mcu_op_t;
  void onFileUpload_mcu(void)
  {
    serverObj->onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                         {
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len); });
  }
  void onRequestBody_mcu(void)
  {
    serverObj->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                          {
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total); });
  }
  typedef std::tuple<String> onRequestBody_mcu_op_t;
  void onNotFound_mcu(void)
  {
    serverObj->onNotFound([](AsyncWebServerRequest *request)
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
  }
  void netUdp_onPacket(void)
  {
    // netudp.listenMulticast(IPAddress(239, 1, 2, 3), 1234);//将UDP套接字绑定到指定的多播组地址和端口，以便接收发送到该多播组地址和端口的UDP数据报
    // netudp.connect(IPAddress(192, 168, 1, 100), 1234);//将UDP套接字连接到指定的目标IP地址和端口
    // netudp.listen(1234);//将UDP套接字绑定到本地的1234端口，以便接收从该端口发送到设备的UDP数据报
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    netudp.onPacket([](AsyncUDPPacket packet)
                    {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            packet.printf("Got %u bytes of data", packet.length()); });
    // Send multicast
    // udp.print("Hello!");
  }
  void netUdp_send(const char *msg)
  {
    netudp.broadcast(msg);
  }
  void netUdp_send(const char *msg, uint16_t port)
  {
    netudp.broadcastTo(msg, port);
  }
};
#endif