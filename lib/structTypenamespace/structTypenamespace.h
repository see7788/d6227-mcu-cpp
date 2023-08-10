#ifndef structTypenamespace_h
#define structTypenamespace_h
#include <freertos/task.h>
#include <ArduinoJson.h>
#include <Arduino.h>

namespace structTypenamespace
{
    typedef struct
    {
        const char *sendTo_name;
        String msg;
    } notifyString_t;
    // typedef struct
    // {
    //     const char *sendTo_name;
    //     char msg[3000];
    // } notifyChar3000_t;
    typedef struct
    {
        const char *sendTo_name;
        JsonArray &msg;
    } notifyJsonArray_t;
}
#endif