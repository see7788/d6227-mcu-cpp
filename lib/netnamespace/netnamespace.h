#ifndef netnamespace_h
#define netnamespace_h
#include <dz003namespace.h>
#include <tuple>
#include <WiFi.h>
namespace netnamespace
{
    typedef struct
    {
        std::string init;
        std::tuple<std::string> ap;
        std::tuple<std::string, std::string> sta;
    } initParam_t;
    void init(const initParam_t &param)
    {
        std::string c = param.init;
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