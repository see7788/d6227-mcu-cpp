#ifndef MyClient_h
#define MyClient_h
#include <Arduino.h>
#include <esp_log.h>
#include <tuple>
#include <functional>
#include <myStruct_t.h>
#include <ArduinoWebsockets.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
class MyClient
{
  bool wsIpcIng;
public:
  websockets::WebsocketsClient wsObj;
  typedef std::tuple<websockets::WSInterfaceString, websockets::WSInterfaceString> wscConfig_t;
  MyClient() :wsObj(websockets::WebsocketsClient()), wsIpcIng(false) {}
  void ws_send(String str)
  {
    if (wsIpcIng) {
      wsObj.send(str);
    }
    else {
      ESP_LOGV("ETBIG", "wsObjIpcIng is false");
    }
  }
  typedef struct
  {
    wscConfig_t& config;
    std::function<void(String)> msgCallBack;
  } wsTaskParam_t;
  void wsTask(void* ptr)
  {
    wsTaskParam_t* c = (wsTaskParam_t*)ptr;
    auto connect = [this]() {
      bool connected = wsObj.connect("39.97.216.195", 6014, "/");
      if (connected) {
        wsObj.send("Hello Server");
        Serial.println("Connected success");
      }
      else {
        Serial.println("Not Connected error");
      }
      };
    wsObj.onMessage([c](websockets::WebsocketsMessage message)
      {
        c->msgCallBack(message.data());
      });
    wsObj.onEvent([c, this, connect](websockets::WebsocketsEvent event, String data)
      {
        if (event == websockets::WebsocketsEvent::ConnectionOpened)
        {
          wsIpcIng = true;
        }
        else if (event == websockets::WebsocketsEvent::ConnectionClosed)
        {
          wsIpcIng = false;
          wsObj.close();
          vTaskDelay(1000);
          connect();
        }
      });
    connect();
    while (true)
    {
      if (wsObj.available())
      {
        wsObj.poll();
      }
      vTaskDelay(500);
    }
  }

};

#endif