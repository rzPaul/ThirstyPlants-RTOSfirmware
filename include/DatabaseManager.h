// DatabaseManager is responsible of connecting and managing Database logic

#pragma once

#include <Arduino.h>
#include "Secrets.h"
#include <Firebase_ESP_Client.h>




extern char g_macAddress[13];

class DatabaseManager
{
private:
    FirebaseConfig _config;
    FirebaseAuth _auth;
    
    FirebaseData _fbdo;
    FirebaseData _streamFbdo;

    FirebaseJson _json;

    TaskHandle_t _databaseTaskHandle = nullptr;

// Arrays of the Database's paths
    char _databaseUserPath[128] = {0};  // users/<databaseUID>
    char _databaseDevicePath[192] = {0}; // users/<databaseUID>/<deviceMACAdress>
    char _databasePingPath[192] = {0}; // users/<databaseUID>/<deviceMACAdress>/online
    char _databaseSoilMoisturePath[192] = {0}; // users/<databaseUID>/<deviceMACAdress>/moistureSensor/<moistureSensorValue>
    char _databaseCommandsPath[192] = {0}; // users/<databaseUID>/<deviceMACAdress>/commands
    bool _isAuthenticated = false;

    unsigned long _lastUploadTimeMS = 0;
    static constexpr unsigned long UPLOAD_INTERVAL_MS = 10000; //debug


    static void s_databaseTask(void *arg);

    void setupFirebase(); 

    void uploadHeartbeat(); // Does nothing atp, should upload periodical timestamps

    void listenForDatabaseCommands(); // Listens for changes in the Database

    void executeFactoryReset(); // Deletes the saved WiFi credentials and restarts the ESP so it goes through the BLE and WiFi configurations again.


public:

    void pingRTDB(); // Pings the Realtime Database 

    void uploadSensorData(int sensorValue);

    void beginTask();
};
