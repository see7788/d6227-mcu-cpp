#ifndef MyWebClient_h
#define MyWebClient_h
#include <Arduino.h>
#include <esp_log.h>
#include <tuple>
#include <functional>
#include <myStruct_t.h>
#include <ArduinoWebsockets.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
class MyWebClient
{
public:
  websockets::WebsocketsClient obj;
  typedef std::tuple<websockets::WSInterfaceString, websockets::WSInterfaceString> config_t;
  MyWebClient() :obj(websockets::WebsocketsClient()) {}
  // void wsClient_send(String str)
  // {
  //   if (globalConfig["wsClient"]["init"].as<bool>())
  //     wsClient.send(str);
  // }
  void wsClient_connect(void)
  {
    // obj.connect(globalConfig["wsClient"]["uri"].as<String>());
    // globalConfig["wsClient"]["Connection"] = true;
    // obj->connect("39.97.216.195",6014,"/");
  }
  typedef struct
  {
    config_t& config;
    std::function<void(void)> callBack;
  } taskParam_t;
  void wsClientTask(void* pt)
  {
    obj.onMessage([](websockets::WebsocketsMessage message)
      {
        // wsClientOnMsg.str = message.data();
        // wsClientOnMsg.send = wsClient_send;
        // xTaskNotify(onMsgHandle, (uint32_t)&wsClientOnMsg, eSetValueWithOverwrite);
      });
    obj.onEvent([this](websockets::WebsocketsEvent event, String data)
      {
        const char* event_data = "";
        if (event == websockets::WebsocketsEvent::ConnectionOpened)
        {
          // globalConfig["wsClient"]["Connection"] = true;
          // ESP_LOGD("debug", "ConnectionOpened");
        }
        else if (event == websockets::WebsocketsEvent::ConnectionClosed)
        {
          // globalConfig["wsClient"]["Connection"] = false;
          // ESP_LOGD("debug", "ConnectionClosed");
          // wsClient->close();
          // vTaskDelay(2888);
          obj.connect("39.97.216.195", 6014, "/");
        }
        // else if (event == websockets::WebsocketsEvent::GotPing)
        // {
        //   ESP_LOGD("debug", "Got a Ping!");
        // }
        // else if (event == websockets::WebsocketsEvent::GotPong)
        // {
        //   ESP_LOGD("debug", "Got a Pong!");
        // }
      });
    obj.connect("39.97.216.195", 6014, "/");
    while (true)
    {
      if (obj.available())
      {
        obj.poll();
      }
      vTaskDelay(455);
    }
  }

};

#endif