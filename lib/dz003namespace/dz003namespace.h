#ifndef dz003namespace_h
#define dz003namespace_h
#include <ETH.h> //引用以使用ETH
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <tuple>
#define ETH_ADDR 1
#define ETH_POWER_PIN -1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
namespace dz003namespace
{

    void eth_begin()
    {
        ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT);
    }
    // 水通断控制
    struct fa_t
    {
        int gpio;
        bool working;
        void set(bool c)
        {
            digitalWrite(gpio, c ? HIGH : LOW);
            working = c;
        }
        fa_t(int gpio) : gpio(gpio)
        {
            pinMode(gpio, OUTPUT);
        }
    };
    // 喇叭控制
    struct laba_t
    {
        int gpio;
        bool working;
        void set(bool c)
        {
            working = c;
            digitalWrite(gpio, c ? HIGH : LOW);
        }
        laba_t(int gpio) : gpio(gpio)
        {
            pinMode(gpio, OUTPUT);
        }
    };
    // 警示灯
    struct deng_t
    {
        int false_goio;
        int true_goio;
        bool working;
        void set(bool c)
        {
            working = c;
            if (c)
            {
                digitalWrite(true_goio, HIGH);
                digitalWrite(false_goio, LOW);
            }
            else
            {
                digitalWrite(true_goio, LOW);
                digitalWrite(false_goio, HIGH);
            }
        }
        deng_t(int false_goio, int true_goio) : false_goio(false_goio), true_goio(true_goio)
        {
            pinMode(false_goio, OUTPUT);
            pinMode(true_goio, OUTPUT);
        }
    };
    volatile int frequencyvalue[2] = {0, 0};
    void frequencyvalue0_add(void)
    {
        //   noInterrupts ();关闭全局所有中断
        frequencyvalue[0]++;
        //   interrupts ();打开全局所有中断
    }
    void frequencyvalue1_add(void)
    {
        frequencyvalue[1]++;
    }
    struct frequency_t
    {
        int gpio[2];
        bool working;
        int log[2];

        void set0()
        {
            frequencyvalue[0] = 0;
            frequencyvalue[1] = 0;
            log[0] = 0;
            log[1] = 0;
        }
        void set(bool c)
        {
            working = c;
            if (c)
            {
                set0();
                // noInterrupts(); // 关闭全局所有中断
                // interrupts(); // 打开全局所有中断
                // LOW： 当引脚为低电平时触发中断服务程序
                // CHANGE： 当引脚电平发生变化时触发中断服务程序
                // RISING： 当引脚电平由低电平变为高电平时触发中断服务程序
                // FALLING： 当引脚电平由高电平变为低电平时触发中断服务程序
                // attachInterrupt(digitalPinToInterrupt(gpio[0]), frequencyvalue0_add, RISING);
                // attachInterrupt(digitalPinToInterrupt(gpio[1]), frequencyvalue1_add, RISING);
            }
            else
            {
                // detachInterrupt(gpio[0]);
                // detachInterrupt(gpio[1]);
            }
        }
        frequency_t(int c0, int c1)
        {
            gpio[0] = c0;
            gpio[1] = c1;
            pinMode(gpio[0], INPUT);
            pinMode(gpio[1], INPUT);
            // set(true);
        }
    };
    typedef std::tuple<int, int, int, int, String> config_t; // sendTo_name,v0v1abs_c，v0v1absLoop_c，loopNumber_c，set0tick_c
    struct Dz003Class
    {
        fa_t fa;               //(2);
        laba_t laba;           //(16);
        deng_t deng;           //(12, 14);
        frequency_t frequency; //(34, 35);
        Dz003Class(void) : fa(2), laba(16), deng(12, 14), frequency(34, 35) {}
        // 初始化工作状态
        void set(bool c)
        {
            fa.set(c);
            laba.set(!c);
            deng.set(!c);
            frequency.set(c);
        }
        void getState(JsonObject obj)
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
            frvalue.add(frequency.log[0]);
            frvalue.add(frequency.log[1]);

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
            if (api.indexOf(".fa.set") > -1)
            {
                fa.set(arr[1].as<bool>());
            }
            else if (api.indexOf(".frequency.set") > -1)
            {
                frequency.set(arr[1].as<bool>());
            }
            else if (api.indexOf(".laba.set") > -1)
            {
                laba.set(arr[1].as<bool>());
            }
            else if (api.indexOf(".deng.set") > -1)
            {
                deng.set(arr[1].as<bool>());
            }
        }
    };

    typedef struct
    {
        config_t &config;
        Dz003Class obj;
        std::function<void(void)> startCallBack;
        std::function<void(void)> tickCallBack;
    } mainTaskParam_t;
    void mainTask(void *ptr)
    {
        TickType_t tickCount = xTaskGetTickCount();
        mainTaskParam_t *c = (mainTaskParam_t *)ptr;
        Dz003Class *obj = &c->obj;
        int &log_0 = obj->frequency.log[0];
        int &log_1 = obj->frequency.log[1];
        int &c_tick = std::get<0>(c->config);
        int &c_abs = std::get<1>(c->config);
        int &c_tickBig = std::get<2>(c->config);
        int &c_absBig = std::get<3>(c->config);
        String &sendTo = std::get<4>(c->config);
        obj->set(true);
        int pre_abs = 0;
        TickType_t pd_tick = pdMS_TO_TICKS(c_tick);
        TickType_t pd_tickBig = tickCount + pdMS_TO_TICKS(c_tickBig);
        c->startCallBack();
        for (;;)
        {
            log_0 = frequencyvalue[0];
            log_1 = frequencyvalue[1];
            int now_abs = std::abs(log_0 - log_1);
            c->tickCallBack();
            if (std::abs(now_abs - pre_abs) > c_abs)
            {
                obj->set(false);
            }
            else if (now_abs > c_absBig)
            {
                obj->set(false);
            }
            else
            {
                pre_abs = now_abs;
            }
            if (pd_tickBig > tickCount)
            {
                pd_tickBig = tickCount + pdMS_TO_TICKS(c_tickBig);
                obj->frequency.set0();
            }
            vTaskDelayUntil(&tickCount, pd_tick);
        }
    }
}
#endif