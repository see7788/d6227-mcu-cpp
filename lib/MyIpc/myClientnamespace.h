#ifndef myClientnamespace_h
#define myClientnamespace_h
#include <Arduino.h>
#include <esp_log.h>
#include <tuple>
#include <functional>
#include <ArduinoWebsockets.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
namespace myClientnamespace {
  class WsClient
  {
    bool ipcIng;
  public:
    websockets::WebsocketsClient obj;
    typedef std::tuple<String, websockets::WSInterfaceString, websockets::WSInterfaceString> config_t;
    typedef struct
    {
      config_t config;
      std::function<void(void)> startCallBack;
      std::function<void(String)> msgCallBack;
    } param_t;
    param_t* param;
    WsClient(param_t* _param) :obj(websockets::WebsocketsClient()), ipcIng(false), param(_param) {
      obj.onEvent([this](websockets::WebsocketsEvent event, String data)
        {
          if (event == websockets::WebsocketsEvent::ConnectionOpened)
          {
            ipcIng = true;
            param->startCallBack();
            ESP_LOGV("debug", "ipcIng success");
          }
          else if (event == websockets::WebsocketsEvent::ConnectionClosed)
          {
            ipcIng = false;
            ESP_LOGV("debug", "ipcIng false");
          }
        });
      obj.onMessage([this](websockets::WebsocketsMessage message)
        {
          param->msgCallBack(message.data());
        });
    }
    void connect(void) {
      //  if (!ipcIng) {
      ipcIng = obj.connect("39.97.216.195", 6014, "/");
      // if (!ipcIng) {
      //   obj.close();
      //   vTaskDelay(3000);
      //   connect();
      // }
      // }
    }
    void send(String str)
    {
      if (ipcIng) {
        obj.send(str);
      }
      else {
        connect();
        obj.send(str);
      }
    }
  };
  void wsTask(void* ptr)
  {
    WsClient* op = (WsClient*)ptr;
    op->connect();
    while (true)
    {
      if (op->obj.available())
      {
        op->obj.poll();
      }
      vTaskDelay(500);
    }
  }
};

#endif