#ifndef MyBle_h
#define MyBle_h

#include <BLEDevice.h>// 这个头文件包含了与 BLE 设备相关的函数和类。它提供了初始化 BLE 设备、获取扫描对象等功能。
#include <BLEUtils.h>// 这个头文件包含了一些与 BLE 相关的实用函数和宏定义。它提供了一些用于处理 BLE 数据、UUID 转换等功能的辅助函数。
#include <BLEScan.h>// 这个头文件包含了 BLE 扫描相关的函数和类。它定义了 BLEScan 类，用于开启和配置 BLE 扫描，以及获取扫描结果。
#include <BLEAdvertisedDevice.h>// 这个头文件包含了处理广播设备的函数和类。它定义了 BLEAdvertisedDevice 类，用于处理扫描到的广播设备信息，并提供了一些相关的回调函数。
// class MyBleScanCallbacks : public BLEAdvertisedDeviceCallbacks {
//   void onResult(BLEAdvertisedDevice advertisedDevice) {
//     Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
//   }
// };
// class MyBleScan
// {
// private:
//   BLEScan* pBLEScan;
// public:
//   MyBleScan() {
//   }
//   void init() {
//     BLEDevice::init("");
//     pBLEScan = BLEDevice::getScan(); //create new scan
//     pBLEScan->setAdvertisedDeviceCallbacks(new MyBleScanCallbacks());
//     pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
//     pBLEScan->setInterval(100);//扫描间隔
//     pBLEScan->setWindow(99);  // less or equal setInterval value
//   }
// };

// class MyBleServer
// {
// private:

// public:
//   MyBleServer() {
//   }
// };
#endif