#ifndef dz003namespace_h
#define dz003namespace_h
#include <ETH.h> //引用以使用ETH
#include <Arduino.h>
#include <esp_log.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <tuple>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <structTypenamespace.h>
#define ETH_ADDR 1
#define ETH_POWER_PIN -1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18

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
   void eth_begin()
   {
      ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT);
   }
   // 水通断控制
   typedef struct
   {
      int gpio;
      bool working;
   } fa_t;
   fa_t fa;
   void fa_set(bool c)
   {
      digitalWrite(fa.gpio, c ? HIGH : LOW);
      fa.working = c;
   }
   // 脉冲控制//逆向水应该不支持
   typedef struct
   {
      int gpio[2];
      bool working;
      int value[2];
      int log[4];
   } frequency_t;
   frequency_t frequency;
   void frequency_value0Add(void)
   {
      frequency.value[0] += 1;
   }
   void frequency_value1Add(void)
   {
      frequency.value[1] += 1;
   }
   void frequency_set(bool c)
   {
      frequency.working = c;
      int v0 = frequency.gpio[0];
      int v1 = frequency.gpio[1];
      if (c)
      {
         // LOW： 当引脚为低电平时触发中断服务程序
         // CHANGE： 当引脚电平发生变化时触发中断服务程序
         // RISING： 当引脚电平由低电平变为高电平时触发中断服务程序
         // FALLING： 当引脚电平由高电平变为低电平时触发中断服务程序
         attachInterrupt(digitalPinToInterrupt(v0), frequency_value0Add, RISING);
         attachInterrupt(digitalPinToInterrupt(v1), frequency_value1Add, RISING);
      }
      else
      {
         detachInterrupt(v0);
         detachInterrupt(v1);
      }
   }
   void frequency_valueset0(void)
   {
      frequency.value[0] = 0;
      frequency.value[1] = 0;
   }
   // 喇叭控制
   typedef struct
   {
      int gpio;
      bool working;
   } laba_t;
   laba_t laba;
   void laba_set(bool c)
   {
      laba.working = c;
      digitalWrite(laba.gpio, c ? HIGH : LOW);
   }
   // 警示灯
   typedef struct
   {
      int gpio[2];
      bool working;
   } deng_t;
   deng_t deng;
   void deng_set(bool c)
   {
      deng.working = c;
      int dengfalse_goio = deng.gpio[0];
      int dengtrue_goio = deng.gpio[1];
      if (c)
      {
         digitalWrite(dengtrue_goio, HIGH);
         digitalWrite(dengfalse_goio, LOW);
      }
      else
      {
         digitalWrite(dengtrue_goio, LOW);
         digitalWrite(dengfalse_goio, HIGH);
      }
   }

   // 初始化工作状态
   void work_set(bool c)
   {
      if (c)
      {
         fa.gpio = 2;
         laba.gpio = 16;
         frequency.gpio[0] = 34;
         frequency.gpio[1] = 35;
         deng.gpio[0] = 12;
         deng.gpio[1] = 14;
         pinMode(fa.gpio, OUTPUT);
         pinMode(laba.gpio, OUTPUT);
         pinMode(frequency.gpio[0], INPUT); // 初始化引脚
         pinMode(frequency.gpio[1], INPUT);
         pinMode(deng.gpio[0], OUTPUT);
         pinMode(deng.gpio[1], OUTPUT);
      }
      frequency_valueset0();
      fa_set(c);
      frequency_set(c);
      laba_set(!c);
      deng_set(!c);
   }

   void state(JsonObject &obj)
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
      dengread.add(digitalRead(deng.gpio[0]));
      dengread.add(digitalRead(deng.gpio[1]));
   }
   typedef std::tuple<String, int, int, int, int> config_t; // sendTo_name,v0v1abs_c，v0v1absLoop_c，loopNumber_c，set0tick_c
   typedef struct
   {
      config_t *config;
      TaskHandle_t sendTo_taskHandle;
   } taskParam_t;
   void resTask(void *ptr)
   {
      taskParam_t *c = (taskParam_t *)ptr;
      TickType_t ticksCount = xTaskGetTickCount();
      work_set(true);
      int &v0v1abs = frequency.log[0];
      v0v1abs = 0;
      int &v0v1absLoop = frequency.log[1];
      v0v1absLoop = 0;
      int &loopNumber = frequency.log[2];
      loopNumber = 0;
      String sendTo;
      int v0v1abs_c, v0v1absLoop_c, loopNumber_c, set0tick_c;
      std::tie(sendTo, v0v1abs_c, v0v1absLoop_c, loopNumber_c, set0tick_c) = *(c->config);
      ESP_LOGV("DEBUG","SUCCESS");
      for (;;)
      {
         //ESP_LOGV("DEBUG","===");
         loopNumber += 1;
         v0v1abs = abs(frequency.value[0] - frequency.value[1]);
         v0v1absLoop += v0v1abs;
         if (v0v1abs > v0v1abs_c || v0v1absLoop > v0v1abs_c)
         {
            work_set(false);
         }
         structTypenamespace::notifyString_t *obj = new structTypenamespace::notifyString_t{
          .sendTo_name = sendTo,
          .msg = "[\"dz003.State\"]"};
         xTaskNotify(c->sendTo_taskHandle, (uint32_t)obj, eSetValueWithOverwrite);
         frequency_valueset0();
         if (loopNumber > loopNumber_c)
         {
            loopNumber = 0;
            v0v1absLoop = 0;
         }
         vTaskDelayUntil(&ticksCount, pdMS_TO_TICKS(set0tick_c));
      }
   }
};
#endif