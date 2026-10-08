#pragma once

#include <Arduino.h>

#ifdef DEBUG_ENABLED
    #define DEBUG_BEGIN(baud) Serial.begin(baud)
    #define DEBUG_PRINT(text)  Serial.print(text)
    #define DEBUG_PRINTLN(text) Serial.println(text)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_BEGIN(baud) ((void)0)
    #define DEBUG_PRINT(text)  ((void)0)
    #define DEBUG_PRINTLN(text) ((void)0)
#endif