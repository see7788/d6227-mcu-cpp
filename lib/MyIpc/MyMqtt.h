#ifndef MyMqtt_h
#define MyMqtt_h
#include "Arduino.h"
#include <Ethernet.h>
#include <WiFi.h>
#include <PubSubClient.h>
class MyMqtt
{
private:
    uint64_t mac;
    PubSubClient obj;
    void callBackDemo(char* topic, byte* payload, unsigned int length) {
        for (uint8_t i = 0; i < length; i++) {
            Serial.write(payload[i]);
        }
        Serial.println();
    }
public:
    MyMqtt(IPAddress ip, uint16_t port, std::function<void(char* topic, byte* payload, unsigned int length)> callBack) : obj(ip, port) {
        obj.setCallback(callBack);
        mac = ESP.getEfuseMac();
        snprintf(clientId, sizeof(clientId), "%04X%08X", (uint16_t)(mac >> 32), (uint32_t)mac);
    }
    // 连接到 MQTT 代理
    bool connectToBroker(void) {
        return obj.connect(mac);
    }
    // 订阅指定的主题
    bool subscribeTopic(const char* topic) {
        return obj.subscribe(topic);
    }
    // 发布消息到指定主题
    void publishMessage(const char* topic, const char* message) {
        obj.publish(topic, message);
    }
    // 处理 MQTT 消息循环
    void loop() {
        obj.loop();
    }
};

#endif