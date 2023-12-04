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
  public:
    bool ipcIng;
    websockets::WebsocketsClient obj;
    typedef std::tuple<String, websockets::WSInterfaceString, websockets::WSInterfaceString> config_t;
    typedef struct
    {
      config_t config;
      std::function<void(void)> startCallBack;
      std::function<void(String)> msgCallBack;
    } param_t;
    param_t* param;
    void connect(void) {
      ipcIng = obj.connect("39.97.216.195", 6014, "/");
    }
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
            ESP_LOGV("debug", "ipcIng Closed");
            // vTaskDelay(3000);
            // connect();
          }
        });
      obj.onMessage([this](websockets::WebsocketsMessage message)
        {
          param->msgCallBack(message.data());
        });
      param->startCallBack();
    }
    void send(String str)
    {
      if (ipcIng)
        obj.send(str);
      // else
      //   ESP_LOGV("debug", "ipcIng false");
    }
  };
  void wsTask(void* ptr)
  {
    WsClient* op = (WsClient*)ptr;
    op->connect();
    while (true)
    {
      op->send("test");
      if (op->obj.available())
      {
        op->obj.poll();
      }
      vTaskDelay(500);
    }
  }
};

#endif