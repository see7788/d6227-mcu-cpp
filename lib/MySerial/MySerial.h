#ifndef serialnamespace_h
#define serialnamespace_h
#include <iostream>
#include <Arduino.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <HardwareSerial.h>
#include <string>
/*
    es32：
    UART0:RX: GPIO3，TX: GPIO1
    UART1:RX: GPIO9，TX: GPIO10
    */
class MySerial : public HardwareSerial
{
public:
    // 接收转发至,波特率,接收引脚,发送引脚
    typedef std::tuple<String, int, int8_t, int8_t> config_t;
    MySerial(config_t &config)
        : HardwareSerial(std::get<1>(config))
    {
        int8_t rx = std::get<2>(config);
        int8_t tx = std::get<3>(config);
        this->setPins(rx, tx);
    }
    // 接收时间调用onReceive方法
    // 取数据调用readStringUntil('\n')
};

#endif