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
    // typedef struct
    // {
    //     const char *sendTo_name;
    //     char msg[3000];
    // } notifyChar3000_t;

}
#endif