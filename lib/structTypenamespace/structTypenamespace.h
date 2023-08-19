#ifndef structTypenamespace_h
#define structTypenamespace_h
#include <ArduinoJson.h>
#include <Arduino.h>
#include <string>
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
    typedef void (*stdStringParse_t)(notifyString_t&);
    typedef void (*jsonArrayParse_t)(notifyJsonArray_t&);
}
#endif