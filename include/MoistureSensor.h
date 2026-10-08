#pragma once

#include <Arduino.h>

constexpr uint16_t MAX_MOISTURE_RAW_VALUE = 4050;
constexpr uint16_t MIN_MOISTURE_RAW_VALUE = 50;
constexpr uint16_t REAL_MAX_MOISTURE_RAW = 3200;
constexpr uint16_t REAL_MIN_MOISTURE_RAW = 1100;
constexpr uint8_t BUFFER_SAMPLE_COUNT = 5;

enum class MoistureSensorError : uint8_t
{
    OK = 0,
    DISCONNECTED,
    SHORT_CIRCUIT
};

class MoistureSensor
{
private:
    uint8_t _sensorPin;
    uint16_t _rawMoistureBuffer[BUFFER_SAMPLE_COUNT];
    uint8_t _bufferIndex;
    uint16_t _rawMoistureFiltered;
    uint8_t _moisturePercent;
    uint32_t _bufferTimerMS;


public:
    MoistureSensor(uint8_t sensorPin);

    void begin();

    bool readAndProcess();

    bool checkTimer(uint32_t intervalToCheckMS, uint32_t &lastCheckMS);

    bool checkMoistureValue();

    static uint8_t convertMoistureRawToPercent(uint16_t moistureValueRaw);
    
    uint8_t getMoisturePercent();
};