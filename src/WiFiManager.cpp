#include "WiFiManager.h"
#include "Secrets.h"
#include "Debug.h"

void WiFiManager::startWifiTask(void *arg)
{
    WiFiManager *_manager = static_cast<WiFiManager *>(arg);
    // We create the name of the device that the router is seeing (ThirstyPlants:<MAC>)
    snprintf(_manager->_networkDisplayId, sizeof(_manager->_networkDisplayId), "ThirstyPlants:%s", g_macAddress);

    WiFi.mode(WIFI_STA);
    
    WiFi.setHostname(_manager->_networkDisplayId);

    _manager->connectToWiFi();

    if (_manager->isConnected())
    {
        _manager->_databaseManager->beginTask();
    }

    vTaskDelete(nullptr);
}

void WiFiManager::connectToWiFi()
{
    WiFi.disconnect(true);
    WiFi.begin(_wifiSSID, _wifiPass);

    uint8_t attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 40)
    {
        attempts++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if (_isSavedWiFi == false)
        {
            saveCredentials();
            _isSavedWiFi = true;
        }
    }
    else
    {
        // debug
    }
}
void WiFiManager::setDatabase(DatabaseManager *databaseManager)
{
    _databaseManager = databaseManager;
}

void WiFiManager::setCredentials(const char *ssid, const char *password)
{
    strlcpy(_wifiSSID, ssid, sizeof(_wifiSSID));
    strlcpy(_wifiPass, password, sizeof(_wifiPass));

    DEBUG_PRINTLN(_wifiPass); // debug
    DEBUG_PRINTLN(_wifiSSID); // debug
}

void WiFiManager::beginTask()
{
    if (_wifiTaskHandle != nullptr)
     return;

    xTaskCreate(
        startWifiTask,
        "WiFi Task",
        4096,
        this,
        1,
        &_wifiTaskHandle);
}

void WiFiManager::saveCredentials()
{
    _preferences.begin("wifi", false);
    _preferences.putString("ssid", _wifiSSID);
    _preferences.putString("pass", _wifiPass);
    _preferences.end();
}

bool WiFiManager::loadCredentials()
{
    _preferences.begin("wifi", true);

    size_t ssidLen = _preferences.getString("ssid", _wifiSSID, sizeof(_wifiSSID));
    size_t passLen = _preferences.getString("pass", _wifiPass, sizeof(_wifiPass));

    _preferences.end();

    if (ssidLen > 0)
    {
        _isSavedWiFi = true;
        return true;
    }

    _isSavedWiFi = false;
    return false;
}

void WiFiManager::clearCredentials()
{
    _preferences.begin("wifi", false);
    _preferences.clear();
    _preferences.end();
}

bool WiFiManager::isConnected()
{
    return (WiFi.status() == WL_CONNECTED);
}