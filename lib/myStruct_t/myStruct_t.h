#ifndef myStruct_t_h
#define myStruct_t_h
#include <ArduinoJson.h>
#include <Arduino.h>
#include <functional>
typedef struct
{
    String sendTo_name;
    String str;
} myStruct_t;
#endif