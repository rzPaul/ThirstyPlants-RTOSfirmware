// BleManager is responsible for starting and stopping BLE communications. It listens for 
// the WiFi credentials (SSID and Password) from a phone, passes them to WiFiManager, waits for 
// a connection, and if it's successful, it sends a confirmation to the phone. 
// Afterwards, it stops the BLE and the task deletes itself so the RAM is freed.
#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "Secrets.h"
#include "WiFiManager.h"

// The separator chosen to delimit the SSID from password since they come as a whole character array 
// Must match the one chosen in the provisioning app
constexpr const char* CREDENTIALS_SEPARATOR = ";;";

extern char g_deviceName[64];

class BleManager
{
private:
    volatile bool _bleStarted = false;
    volatile bool _newCredentialsReceived = false;
    char _receivedCredentials[128] = {0};

    TaskHandle_t _bleTaskHandle = nullptr;

    WiFiManager *_wifiManager = nullptr;
    BLEServer *_pServer = nullptr;
    BLECharacteristic *_pCharacteristic = nullptr;
    BLECharacteristicCallbacks *_callbacks = nullptr;

    static void startBleTask(void *arg);

    class CredentialsCallbacks : public BLECharacteristicCallbacks
    {
    private:
        BleManager *_manager;
    public:
        // A constructor that keeps a pointer to the owning BleManager so onWrite() can reach it.
        CredentialsCallbacks(BleManager *manager);
        // Called by the BLE stack when the phone writes credentials.
        void onWrite(BLECharacteristic *pCharacteristic);     
        // Splits the recieved string into SSID and password and starts WiFi.
        void parseCredentials();  
    };

    void startBLE();
    void stopBLE();
    
public:
    // Sets the WiFiManager that will recieve the credentials.
    // Must be called before beginTask() during the setup!
    void setWiFi(WiFiManager *wifiManager); 
    // Starts BLE provisioning task. So we don't start multiple task that do the same thing, it only proceeds if _bleStarted == FALSE.
    void beginTask();
    // Returns true since the BLE has been started untill it has been shut down.
    bool isBleStarted();
};
