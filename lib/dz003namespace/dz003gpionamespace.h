#ifndef dz003gpionamespace_h
#define dz003gpionamespace_h
#include <ETH.h> //引用以使用ETH
#include <Arduino.h>
#define ETH_ADDR 1
#define ETH_POWER_PIN -1
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
namespace dz003gpionamespace
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
    // 脉冲控制//逆向水应该不支持
    typedef int frequencyvalue_namespaceprive_t[2];
    frequencyvalue_namespaceprive_t frequencyvalue_namespaceprive;
    void frequencyvalue0_add(void)
    {
        frequencyvalue_namespaceprive[0] += 1;
    }
    void frequencyvalue1_add(void)
    {
        frequencyvalue_namespaceprive[1] += 1;
    }
    struct frequency_t
    {
        int gpio[2];
        bool working;
        frequencyvalue_namespaceprive_t &value = frequencyvalue_namespaceprive;
        int log[3];
        struct
        {
            char v0v1abs;
            char v0v1absLoop;
            char loopNumber;
        } log_index = {0, 1, 2};
        void logset0(void)
        {
            log[0] = 0;
            log[1] = 0;
            log[2] = 0;
        }
        void set(bool c)
        {
            working = c;
            if (c)
            {
                logset0();
                // LOW： 当引脚为低电平时触发中断服务程序
                // CHANGE： 当引脚电平发生变化时触发中断服务程序
                // RISING： 当引脚电平由低电平变为高电平时触发中断服务程序
                // FALLING： 当引脚电平由高电平变为低电平时触发中断服务程序
                attachInterrupt(digitalPinToInterrupt(gpio[0]), frequencyvalue0_add, RISING);
                attachInterrupt(digitalPinToInterrupt(gpio[1]), frequencyvalue1_add, RISING);
            }
            else
            {
                detachInterrupt(gpio[0]);
                detachInterrupt(gpio[1]);
            }
        }
        void valueset0(void)
        {
            value[0] = 0;
            value[1] = 0;
        }
        frequency_t(int c0, int c1)
        {
            gpio[0] = c0;
            gpio[1] = c1;
            valueset0();
            pinMode(gpio[0], INPUT);
            pinMode(gpio[1], INPUT);
        }
    };

}
#endif