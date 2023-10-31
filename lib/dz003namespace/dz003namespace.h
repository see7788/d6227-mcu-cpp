#ifndef dz003namespace_h
#define dz003namespace_h
#include <Arduino.h>
#include <esp_log.h>
#include <ArduinoJson.h>
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
   typedef struct
   {
      config_t &config;
      QueueHandle_t &parseStringQueueHandle;
      std::function<void(void)> startCallback;
   } taskParam_t;

   using dz003gpionamespace::eth_begin;
   dz003gpionamespace::fa_t fa(2);
   dz003gpionamespace::laba_t laba(16);
   dz003gpionamespace::deng_t deng(12, 14);
   dz003gpionamespace::frequency_t frequency(34, 35);

   // 初始化工作状态
   void work_set(bool c)
   {
      fa.set(c);
      frequency.set(c);
      laba.set(!c);
      deng.set(!c);
   }

   void getState(JsonObject &obj)
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
      frvalue.add(frequency.value[0]);
      frvalue.add(frequency.value[1]);
      JsonArray frlog = c2.createNestedArray("log");
      frlog.add(frequency.log[0]);
      frlog.add(frequency.log[1]);
      frlog.add(frequency.log[2]);
      frlog.add(frequency.log[3]);

      JsonObject c3 = obj.createNestedObject("laba");
      c3["working"] = laba.working;
      c3["digitalRead"] = digitalRead(laba.gpio);

      JsonObject c4 = obj.createNestedObject("deng");
      c4["working"] = deng.working;
      JsonArray dengread = c4.createNestedArray("digitalRead");
      dengread.add(digitalRead(deng.false_goio));
      dengread.add(digitalRead(deng.true_goio));
   }

   void resTask(void *ptr)
   {
      taskParam_t *c = (taskParam_t *)ptr;
      TickType_t ticksCount = xTaskGetTickCount();
      int &v0v1abs_log = frequency.log[frequency.log_index.v0v1abs];
      int &v0v1absLoop_log = frequency.log[frequency.log_index.v0v1absLoop];
      int &loopNumber_log = frequency.log[frequency.log_index.loopNumber];
      String &sendTo = std::get<0>(c->config);
      int &v0v1abs_c = std::get<1>(c->config);
      int &v0v1absLoop_c = std::get<2>(c->config);
      int &loopNumber_c = std::get<3>(c->config);
      int &set0tick_c = std::get<4>(c->config);
      ESP_LOGV("DZ003", "SUCCESS");
      c->startCallback();
      work_set(true);
      while (1)
      {
         loopNumber_log += 1;
         v0v1abs_log = abs(frequency.value[0] - frequency.value[1]);
         v0v1absLoop_log += v0v1abs_log;
         if (v0v1abs_log > v0v1abs_c || v0v1abs_log > v0v1absLoop_c || v0v1absLoop_log > v0v1absLoop_c)
         {
            work_set(false);
         }
         if (sendTo)
         {
            myStruct_t obj = {
                .sendTo_name = sendTo,
                .str = "[\"mcu_dz003State_get\"]"};
            if (xQueueSend(c->parseStringQueueHandle, &obj, 0) != pdPASS)
            {
               ESP_LOGV("DZ003", "Queue is full");
            }
         }
         frequency.valueset0();
         if (loopNumber_log > loopNumber_c)
         {
            loopNumber_log = 0;
            v0v1absLoop_log = 0;
         }
         vTaskDelayUntil(&ticksCount, pdMS_TO_TICKS(set0tick_c));
      }
   }
};
#endif