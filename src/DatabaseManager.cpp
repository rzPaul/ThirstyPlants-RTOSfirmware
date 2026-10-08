#include "DatabaseManager.h"
#include "Secrets.h"
#include <Preferences.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include "Debug.h"

void DatabaseManager::s_databaseTask(void *arg)
    {
        DatabaseManager *_manager = static_cast<DatabaseManager *>(arg);

        _manager->setupFirebase();
        
        while(true)
        {
            if(_manager -> _isAuthenticated && Firebase.ready())
            {
                if (millis() - _manager -> _lastUploadTimeMS >= UPLOAD_INTERVAL_MS)
                {
                    _manager -> pingRTDB();
                    _manager -> uploadSensorData(DUMMY_SENSOR_VALUE);
                    _manager -> _lastUploadTimeMS = millis();
                } 
                
                _manager -> listenForDatabaseCommands();
            }

            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }

void DatabaseManager::setupFirebase()
    {
        _config.api_key = API_KEY;
        _config.database_url = FIREBASE_HOST;

        _auth.user.email = DEMO_FIREBASE_EMAIL;
        _auth.user.password = DEMO_FIREBASE_PASSWORD;

        _config.token_status_callback = tokenStatusCallback;
        Firebase.begin(&_config, &_auth);
        Firebase.reconnectNetwork(true); 

        DEBUG_PRINTLN("Authenticating"); // debug
        
        uint8_t retries = 0;

        while (_auth.token.uid == "" && retries < 40)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
            retries++;
            DEBUG_PRINT("."); // debug
        }

        DEBUG_PRINT("Authenticated! Logged into UID: "); // debug
        DEBUG_PRINT(_auth.token.uid.c_str());            // debug

        if (_auth.token.uid != "" && Firebase.ready())
        {

            DEBUG_PRINTLN("\n Authenticated successfully!"); // debug
            DEBUG_PRINT(" Device UID: ");                    // debug
            DEBUG_PRINTLN(_auth.token.uid.c_str());          // debug

            _isAuthenticated = true;

// Parse the Realtime Database paths
            snprintf(_databaseUserPath, sizeof(_databaseUserPath), "users/%s", _auth.token.uid.c_str()); 
            snprintf(_databaseDevicePath, sizeof(_databaseDevicePath), "%s/%s", _databaseUserPath, g_macAddress);
            snprintf(_databasePingPath, sizeof(_databasePingPath), "%s/%s", _databaseDevicePath, "online");
            snprintf(_databaseSoilMoisturePath, sizeof(_databaseSoilMoisturePath), "%s/%s", _databaseDevicePath, "soilMoisture");
            snprintf(_databaseCommandsPath, sizeof(_databaseCommandsPath),"%s/%s", _databaseDevicePath, "commands");
        
            Firebase.RTDB.beginStream(&_streamFbdo, _databaseCommandsPath);
        }
        
    }

    void DatabaseManager::listenForDatabaseCommands()
    {
        if(!Firebase.RTDB.readStream(&_streamFbdo))
        {
            return;
        }

        if(_streamFbdo.streamTimeout())
        {
            return;
        }

        if(_streamFbdo.streamAvailable())
        {
            if(_streamFbdo.dataType() == "json")
            {
                FirebaseJson &json = _streamFbdo.jsonObject();
                FirebaseJsonData jsonResult;

                json.get(jsonResult, "reset");

                if(jsonResult.success && jsonResult.boolValue == true)
                {
                    executeFactoryReset();
                }
            }
            else if (_streamFbdo.dataType() == "boolean")
            {
                if (_streamFbdo.dataPath() == "/reset" && _streamFbdo.boolData() == true)
                {
                    executeFactoryReset();
                }
            }
        }
    }

    void DatabaseManager::executeFactoryReset()
    {
        Preferences prefs;
        prefs.begin("wifi", false);
        prefs.clear();
        prefs.end();

        delay(500);

        ESP.restart();
    }

    void DatabaseManager::pingRTDB()
    {
        if (Firebase.RTDB.setBool(&_fbdo, _databasePingPath, true))
        {
            DEBUG_PRINTLN("Succes"); // debug
        }
        else
        {
            DEBUG_PRINTLN(_fbdo.errorReason()); // debug
        }
    }


    void DatabaseManager::uploadSensorData(int sensorValue)
    {
        Firebase.RTDB.setInt(&_fbdo, _databaseSoilMoisturePath, sensorValue);
    }

    void DatabaseManager::beginTask()
    {
        if(_databaseTaskHandle != nullptr)
            return;

        xTaskCreate(
            s_databaseTask,
            "Database Task",
            8192,
            this,
            1,
            &_databaseTaskHandle);
    }