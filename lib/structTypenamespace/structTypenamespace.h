#ifndef structTypenamespace_h
#define structTypenamespace_h
#include <ArduinoJson.h>
#include <Arduino.h>
#include <functional>
namespace structTypenamespace
{
    typedef struct
    {
        String sendTo_name;
        String msg;
    } myString_t;
    typedef struct
    {
        String sendTo_name;
        JsonArray msg;
    } myJsonArray_t;
    // typedef std::function<void(notifyString_t &)> stdStringParse_t;
    // typedef std::function<void(notifyJsonArray_t &)> jsonArrayParse_t;
};
#endif