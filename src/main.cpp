#include <Arduino.h>
#include <WiFi.h>
#include <BoardPins.h>
#include "DatabaseManager.h"
#include "SensorsManager.h"
#include "BleManager.h"
#include "ButtonManager.h"
#include "Secrets.h"
#include "PumpManager.h"

//Global instances so they can be used across different FreeRTOS tasks
SemaphoreHandle_t g_pumpWakeSemaphore = nullptr;
BleManager g_BLE; 
ButtonManager g_Button; 
WiFiManager g_WiFi; 
DatabaseManager g_Database; 
PumpManager g_Pump;
SensorsManager g_Sensors; 

char g_macAddress[13] = {0}; // Global MAC address of the device
char g_deviceName[64] = {0}; // Global name of the device

// Sets the device name globally so the user can identify their devices in the app or when 
// seraching for it via BLE.
void setDeviceName()
{
    snprintf(g_deviceName, sizeof(g_deviceName), "%s", "DefaultDeviceName");
}

// Gets the MAC address globally and formats it for use as a display identifier or database ID.
void getMacAdress()
{
    uint8_t baseMacAdress[6];

    esp_read_mac(baseMacAdress, ESP_MAC_WIFI_STA);

    snprintf(g_macAddress, sizeof(g_macAddress), "%02X%02X%02X%02X%02X%02X",
    baseMacAdress[0], baseMacAdress[1], baseMacAdress[2], baseMacAdress[3], baseMacAdress[4], baseMacAdress[5]);

    DEBUG_PRINTLN(g_macAddress);
}

void setup()
{
// Initializing the the system componets and FreeRTOS tasks

    Serial.begin(115200);

    getMacAdress();
    setDeviceName();

    g_pumpWakeSemaphore = xSemaphoreCreateBinary();

    g_Button.setupButton(BLE_BUTTON_PIN, &g_BLE);
    g_Button.beginTask();
    
    g_Pump.setSensorsManager(&g_Sensors);
    g_Pump.beginTask();
    
    g_Sensors.beginTask();
    
    g_WiFi.setDatabase(&g_Database);

    g_BLE.setWiFi(&g_WiFi);

// Checks if there are WiFi credentials available, that had been previously saved. If there are, it connects to WiFi, 
// otherwise it starts the BLE task and searches for a phone to connect to.
    if (g_WiFi.loadCredentials())
    {
        g_WiFi.beginTask();
    }
    else
    {
        g_BLE.beginTask();
    }

}

void loop()
{
    // Since system logic is handled entirely by the FreeRTOS tasks we initialised in setup(),
    // we no longer need the Arduino loop() task, so we delete it and reclaim the RAM 
    vTaskDelete(nullptr);
}