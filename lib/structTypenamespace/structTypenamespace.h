#ifndef structTypenamespace_h
#define structTypenamespace_h
#include <ArduinoJson.h>
#include <Arduino.h>
#include <string>
#include <functional>
namespace structTypenamespace
{
    typedef struct
    {
        std::string sendTo_name;
        String msg;
    } notifyString_t;
    typedef struct
    {
        std::string sendTo_name;
        JsonArray msg;
    } notifyJsonArray_t;
    typedef std::function<void(notifyString_t &)> stdStringParse_t;
    typedef std::function<void(notifyJsonArray_t &)> jsonArrayParse_t;
    typedef std::function<void(void)> callback;
}
#endif