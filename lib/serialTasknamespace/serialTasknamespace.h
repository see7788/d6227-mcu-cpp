#ifndef serialTasknamespace_h
#define serialTasknamespace_h
#include <iostream>
#include <Arduino.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <structTypenamespace.h>
#include <HardwareSerial.h>
#include <string>
#include <structTypenamespace.h>
namespace serialTasknamespace
{
    /*
    es32：
    UART0:RX: GPIO3，TX: GPIO1
    UART1:RX: GPIO9，TX: GPIO10
    */
    // 波特率,接收引脚
    // typedef std::tuple<int, int> config_t;
    // typedef struct
    // {
    //     config_t config;
    // } param_t;

    // class MySerial : public HardwareSerial
    // {
    // public:
    //     MySerial(param_t &param) : HardwareSerial(std::get<1>(param.config))
    //     {
    //         _rxPin = std::get<2>(param.config) ? std::get<2>(param.config) : 3 ;
    //         pinMode(_rxPin, INPUT);
    //     }
    //     // 接收时间调用onReceive方法
    //     // 取数据调用readStringUntil('\n')

    //     setTxGpio(int c)
    //     {
    //         pinMode(c, OUTPUT);
    //     }
    // };

    // Serial.onReceive([]()
    //                  {
    //  structTypenamespace::notifyString_t strObj = {
    //   globalConfig["server"]["serial"][0].as<const char *>(),
    //   Serial.readStringUntil('\n')
    //   };
    //    parseSend(&strObj); });

    // typedef struct
    // {
    //     // int *baudRate;
    //     // int *rxGpio;
    //     const char *sendTo_name;
    //     TaskHandle_t sendTo_taskHandle;
    // } onTaskParam_t;

    // SemaphoreHandle_t SerialSemaphore_Handle;
    // QueueHandle_t SerialQueue_Handle;
    // void IRAM_ATTR onTask_serialEvent()
    // {
    //     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    //     String *data = new String(Serial.readStringUntil('\n'));
    //     xQueueSendFromISR(SerialQueue_Handle, &data, &xHigherPriorityTaskWoken);
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }

    // void onTask(void *funparam)
    // {
    //     onTaskParam_t param = *(onTaskParam_t *)funparam;
    //     structTypenamespace::notifyString_t obj = {param.sendTo_name};
    //     SerialQueue_Handle = xQueueCreate(3000, sizeof(String *));
    //     attachInterrupt(digitalPinToInterrupt(3), onTask_serialEvent, FALLING);
    //     while (true)
    //     {
    //         String *receivedData;
    //         if (xQueueReceive(SerialQueue_Handle, &receivedData, portMAX_DELAY) == pdTRUE)
    //         {
    //             obj.msg = *receivedData;
    //             xTaskNotify(param.sendTo_taskHandle, (uint32_t)&obj, eSetValueWithOverwrite);
    //             delete receivedData;
    //         }
    //     }
    // }
    // serialTasknamespace::onTaskParam_t server_serial_Param = {globalConfig["server"]["serial"][0].as<const char *>(), resStr_TaskHandle};
    // xTaskCreate(serialTasknamespace::onTask, "server_serial_Task", 1024 * 4, (void *)&server_serial_Param, taskindex++, &server_serial_TaskHandle);

    // 要在 ESP32 上使用特定的硬件发送数据，您可以使用 ESP32 的 GPIO/I2C/SPI 等接口来与特定硬件进行通信。下面是几个常见的例子：

    // GPIO 发送：如果您需要通过 GPIO 引脚发送数据，您可以使用 digitalWrite 函数将引脚设置为输出模式，并使用 digitalWrite 函数将引脚状态设置为高电平或低电平来发送数据。例如：
    // cpp
    // pinMode(outputPin, OUTPUT); // 将引脚设置为输出模式
    // digitalWrite(outputPin, HIGH); // 发送高电平
    // I2C 发送：如果您需要通过 I2C 接口发送数据，首先需要初始化 I2C 总线并设置从设备地址，然后使用 Wire.beginTransmission() 和 Wire.write() 函数发送数据。例如：
    // cpp
    // Wire.begin(); // 初始化 I2C 总线
    // Wire.beginTransmission(address); // 设置从设备地址
    // Wire.write(data); // 发送数据
    // Wire.endTransmission(); // 结束传输
    // SPI 发送：如果您需要通过 SPI 接口发送数据，首先需要初始化 SPI 总线并设置通信参数，然后使用 SPI.transfer() 函数发送数据。例如：
    // cpp
    // SPI.begin(); // 初始化 SPI 总线
    // SPI.beginTransaction(SPISettings(speed, dataOrder, dataMode)); // 设置通信参数
    // SPI.transfer(data); // 发送数据
    // SPI.endTransaction(); // 结束传输

}

#endif