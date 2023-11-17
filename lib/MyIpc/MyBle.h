#ifndef MyBle_h
#define MyBle_h
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEClient.h>

class MyBle
{
private:
  BLEClient *pClient;
  BLEScan *pScan;
  bool scanning;
public:
  MyBle(){
     scanning = false;
  }
  void init();
  void createService(const char *serviceName);
  void startScanning();
  void stopScanning();
  bool isScanning();
  void sendData(const char *deviceAddress, uint8_t *data, size_t length);

};
void MyBle::init()
{
  BLEDevice::init("");
}

void MyBle::createService(const char *serviceName)
{
  // 创建蓝牙服务
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(serviceName);

  // 添加特征值到服务中
  // 这里需要根据具体需求调用 BLE API 来添加特征值

  pService->start();
  pServer->getAdvertising()->start();
}

void MyBle::startScanning()
{
  // 开始扫描
  if (!scanning)
  {
    pScan = BLEDevice::getScan();
    // pScan->setAdvertisedDeviceCallbacks([](){});
    pScan->setActiveScan(true);
    pScan->start(0, false);
    scanning = true;
  }
}

void MyBle::stopScanning()
{
  // 停止扫描
  if (scanning)
  {
    pScan->stop();
    scanning = false;
  }
}

bool MyBle::isScanning()
{
  // 检查是否正在扫描中
  return scanning;
}

void MyBle::sendData(const char *deviceAddress, uint8_t *data, size_t length)
{
  // 发送数据给指定地址的蓝牙设备
  BLEAddress addr(deviceAddress);
  pClient = BLEDevice::createClient();
  pClient->connect(addr);
  pClient->disconnect();
  delete pClient;
}
#endif