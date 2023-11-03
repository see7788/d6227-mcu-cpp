#ifndef dz003namespace_h
#define dz003namespace_h
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <esp_log.h>
#include <tuple>
#include <freertos/queue.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <myStruct_t.h>
#include <functional>
#include <dz003gpionamespace.h>

/*

char data[3000]
data="abc";
data+="123"
data="";
好像不能这样玩
char data[3000]
data="abc";
memset(&data,0,sizeof(data));
memcpy(&data,(void*)"abc",5);
data+="123"
strcat(&data,(uint8_t*)"123");
data="";
memset(&data,0,sizeof(data));
*/
namespace dz003namespace
{
   typedef std::tuple<String, int, int, int, int> config_t; // sendTo_name,v0v1abs_c，v0v1absLoop_c，loopNumber_c，set0tick_c

   using dz003gpionamespace::eth_begin;
   class Dz003Class
   {
   public:
      dz003gpionamespace::fa_t fa;               //(2);
      dz003gpionamespace::laba_t laba;           //(16);
      dz003gpionamespace::deng_t deng;           //(12, 14);
      dz003gpionamespace::frequency_t frequency; //(34, 35);
      Dz003Class(void) : fa(2), laba(16), deng(12, 14), frequency(34, 35) {}
      // 初始化工作状态
      void work_set(bool c)
      {
         fa.set(c);
         frequency.set(c);
         laba.set(!c);
         deng.set(!c);
      }
      void getState(JsonVariant obj)
      {
         JsonObject c1 = obj.createNestedObject("fa");
         c1["working"] = fa.working;
         c1["digitalRead"] = digitalRead(fa.gpio);

         JsonObject c2 = obj.createNestedObject("frequency");
         c2["working"] = frequency.working;
         JsonArray frread = c2.createNestedArray("digitalRead");
         frread.add(digitalRead(frequency.gpio[0]));
         frread.add(digitalRead(frequency.gpio[1]));
         JsonArray frvalue = c2.createNestedArray("value");
         frvalue.add(frequency.value0);
         frvalue.add(frequency.value1);
         JsonArray frlog = c2.createNestedArray("log");
         frlog.add(frequency.log[0]);
         frlog.add(frequency.log[1]);
         frlog.add(frequency.log[2]);

         JsonObject c3 = obj.createNestedObject("laba");
         c3["working"] = laba.working;
         c3["digitalRead"] = digitalRead(laba.gpio);

         JsonObject c4 = obj.createNestedObject("deng");
         c4["working"] = deng.working;
         JsonArray dengread = c4.createNestedArray("digitalRead");
         dengread.add(digitalRead(deng.false_goio));
         dengread.add(digitalRead(deng.true_goio));
      }
      //[api,bool]
      void res(JsonVariant arr)
      {
         String api = arr[0].as<String>();
         if (api == "mcu_dz003.fa_set")
         {
            fa.set(arr[1].as<bool>());
         }
         else if (api == "mcu_dz003.frequency_set")
         {
            frequency.set(arr[1].as<bool>());
         }
         else if (api == "mcu_dz003.laba_set")
         {
            laba.set(arr[1].as<bool>());
         }
         else if (api == "mcu_dz003.deng_set")
         {
            deng.set(arr[1].as<bool>());
         }
      }
   };
   typedef struct
   {
      config_t &config;
      QueueHandle_t &queueHandle;
      Dz003Class obj;
      std::function<void(void)> startCallback;
   } taskParam_t;
   void looptask(void *ptr)
   {
      TickType_t ticksCount = xTaskGetTickCount();
      taskParam_t *c = (taskParam_t *)ptr;
      Dz003Class *obj = &c->obj;
      String &sendTo = std::get<0>(c->config);
      int &abs_c = std::get<1>(c->config);
      int &loopAbs_c = std::get<2>(c->config);
      int &loopNumber_c = std::get<3>(c->config);
      int &abs_log = obj->frequency.log[obj->frequency.log_index.v0v1abs];
      int &absLoop_log = obj->frequency.log[obj->frequency.log_index.v0v1absLoop];
      int &loopNumber_log = obj->frequency.log[obj->frequency.log_index.loopNumber];
      int &tick_c = std::get<4>(c->config);
      c->startCallback();
      for (;;)
      {
         loopNumber_log++;
         abs_log = abs(obj->frequency.value0 - obj->frequency.value1);
         if (abs_log >= abs_c)
         {
            obj->work_set(false);
         }
         myStruct_t data = {
             .sendTo_name = sendTo,
             .str = "[\"mcu_dz003State_get\"]"};
         if (xQueueSend(c->queueHandle, &data, 0) != pdPASS)
         {
            ESP_LOGV("DZ003", "Queue is full");
         }
         obj->frequency.valueset0();
         vTaskDelayUntil(&ticksCount, pdMS_TO_TICKS(tick_c));
      }
   }
   // 这个不能用，不知道为什么
   void resTask(void *ptr)
   {
      taskParam_t *c = (taskParam_t *)ptr;
      Dz003Class *obj = &c->obj;
      TickType_t ticksCount = xTaskGetTickCount();
      int &v0v1abs_log = obj->frequency.log[obj->frequency.log_index.v0v1abs];
      int &v0v1absLoop_log = obj->frequency.log[obj->frequency.log_index.v0v1absLoop];
      int &loopNumber_log = obj->frequency.log[obj->frequency.log_index.loopNumber];
      String &sendTo = std::get<0>(c->config);
      int &v0v1abs_c = std::get<1>(c->config);
      int &v0v1absLoop_c = std::get<2>(c->config);
      int &loopNumber_c = std::get<3>(c->config);
      int &set0tick_c = std::get<4>(c->config);
      ESP_LOGV("DZ003", "SUCCESS");
      c->startCallback();
      obj->work_set(true);
      for (;;)
      {
         // loopNumber_log += 1;
         // v0v1abs_log = abs(obj->frequency.value0 - obj->frequency.value1);
         // v0v1absLoop_log += v0v1abs_log;
         // if (v0v1abs_log > v0v1abs_c || v0v1abs_log > v0v1absLoop_c || v0v1absLoop_log > v0v1absLoop_c)
         // {
         //    obj->work_set(false);
         // }
         // myStruct_t data = {
         //     .sendTo_name = sendTo,
         //     .str = "[\"mcu_dz003State_get\"]"};
         // if (xQueueSend(c->queueHandle, &data, 0) != pdPASS)
         // {
         //    ESP_LOGV("DZ003", "Queue is full");
         // }
         // obj->frequency.valueset0();
         // if (loopNumber_log > loopNumber_c)
         // {
         //    loopNumber_log = 0;
         //    v0v1absLoop_log = 0;
         // }
         vTaskDelayUntil(&ticksCount, pdMS_TO_TICKS(set0tick_c));
      }
      ESP_LOGV("DZ003", "vTaskDelete");
      vTaskDelete(NULL);
   }
};
#endif