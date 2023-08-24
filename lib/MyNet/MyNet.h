#ifndef MyNet_h
#define MyNet_h
#include <dz003namespace.h>
#include <tuple>
#include <WiFi.h>
class MyNet
{
public:
    typedef struct
    {
        String init;
        std::tuple<String> ap;
        std::tuple<String, String> sta;
    } config_t;
    MyNet(config_t &param)
    {
        String c = param.init;
        if (c == "ap")
        {
            WiFi.mode(WIFI_AP);
            WiFi.softAP(std::get<0>(param.ap).c_str());
        }
        else if (c == "sta")
        {
            WiFi.mode(WIFI_STA);
            WiFi.begin(std::get<0>(param.sta).c_str(), std::get<1>(param.sta).c_str());
        }
        else if (c == "eth")
        {
            dz003namespace::eth_begin();
        }
        else if (c == "ap+sta")
        {
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(std::get<0>(param.ap).c_str());
            WiFi.begin(std::get<0>(param.sta).c_str(), std::get<1>(param.sta).c_str());
        }
        else if (c == "ap+eth")
        {
            WiFi.mode(WIFI_AP);
            WiFi.softAP(std::get<0>(param.ap).c_str());
            dz003namespace::eth_begin();
        }
        else
        {
            ESP_LOGV("debug", "init false");
        }
        ESP_LOGD("end", "%s", c.c_str());
    }
};
#endif