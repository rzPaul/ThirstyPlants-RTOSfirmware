#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <DatabaseManager.h>

extern char g_macAddress[13];

class WiFiManager
{
private:
    char _wifiPass[64] = {0};
    char _wifiSSID[64] = {0};
    char _networkDisplayId[64] = {0};
    bool _isSavedWiFi = false;

    TaskHandle_t _wifiTaskHandle = nullptr;

    Preferences _preferences;

    DatabaseManager *_databaseManager = nullptr;

    static void startWifiTask(void *arg);

public:
    void connectToWiFi();
    // Sets the DatabaseManager so the WiFiManager can use it
    // Must be called before beginTask() during the setup!
    void setDatabase(DatabaseManager *databaseManager);
    // Sets the credentials recieved from BleManager in WiFiManager so it can use them later.
    void setCredentials(const char *ssid, const char *password);
    // Begins WiFi Task
    void beginTask();
    // Save credentials in Preferences so the ESP doesn't erase them when it shuts down.
    void saveCredentials();
    // Loads the saved credentials from Preferences and returns TRUE or FAULSE if successful.
    bool loadCredentials();
    // Clears the credentials from Preferences so it can later reconnect to a new WiFi.
    void clearCredentials();
    // True if connected to WiFi
    bool isConnected();
};