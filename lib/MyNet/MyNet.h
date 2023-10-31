#ifndef MyNet_h
#define MyNet_h
#include <dz003namespace.h>
#include <tuple>
#include <WiFi.h>
class MyNet
{
public:
    typedef String type_t; //""| "ap" | "sta" | "eth" | "ap+sta" | "ap+eth"
    typedef std::tuple<String> ap_t;
    typedef std::tuple<String, String> sta_t;
    typedef std::tuple<String, ap_t, sta_t> config_t;
    MyNet(config_t &config)
    {
        type_t uset;
        ap_t ap;
        sta_t sta;
        std::tie(uset, ap, sta) = config;
        if (uset == "ap")
        {
            WiFi.mode(WIFI_AP);
            WiFi.softAP(std::get<0>(ap).c_str());
        }
        else if (uset == "sta")
        {
            WiFi.mode(WIFI_STA);
            WiFi.begin(std::get<0>(sta).c_str(), std::get<1>(sta).c_str());
        }
        else if (uset == "eth")
        {
            dz003namespace::eth_begin();
        }
        else if (uset == "ap+sta")
        {
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(std::get<0>(ap).c_str());
            WiFi.begin(std::get<0>(sta).c_str(), std::get<1>(sta).c_str());
        }
        else if (uset == "ap+eth")
        {
            WiFi.mode(WIFI_AP);
            WiFi.softAP(std::get<0>(ap).c_str());
            dz003namespace::eth_begin();
        }
        else
        {
            ESP_LOGV("debug", "init bug %s", uset.c_str());
        }
    }
};
#endif