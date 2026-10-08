#include "Secrets.h"
#include "BoardPins.h"
#include "BleManager.h"
#include "Debug.h"

BleManager::CredentialsCallbacks::CredentialsCallbacks(BleManager *manager)
{
    _manager = manager;
}

void BleManager::CredentialsCallbacks::onWrite(BLECharacteristic *pCharacteristic)
{

    strlcpy(_manager->_receivedCredentials, pCharacteristic->getValue().c_str(), sizeof(_manager->_receivedCredentials));
    _manager->_newCredentialsReceived = true;

    DEBUG_PRINTF("WiFI and Pass: %s \n", _manager->_receivedCredentials);

    parseCredentials();
}

void BleManager::CredentialsCallbacks::parseCredentials()
{
    // Because the credentials come in this form "<SSID>;;<password>", they must be parsed in separate variables 
    char *separator = strstr(_manager->_receivedCredentials, CREDENTIALS_SEPARATOR);

    if (separator != nullptr)
    {
        separator[0] = '\0';

        _manager->_wifiManager->setCredentials(_manager->_receivedCredentials, separator + strlen(CREDENTIALS_SEPARATOR));
        _manager->_wifiManager->beginTask();
    }
}

void BleManager::startBleTask(void *arg)
{
    BleManager *_manager = static_cast<BleManager *>(arg);

    _manager->startBLE();

    while (_manager->_wifiManager == nullptr || !_manager->_wifiManager->isConnected())
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (_manager->_pCharacteristic != nullptr)
    {
        DEBUG_PRINTLN("Wi-Fi connected! Sending success confirmation to app..."); // debug

        _manager->_pCharacteristic->setValue("CONNECTED");
        _manager->_pCharacteristic->notify();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelay(pdMS_TO_TICKS(700));

    _manager->stopBLE();

    vTaskDelete(nullptr);
}

void BleManager::startBLE()
{
    BLEDevice::init(g_deviceName);
    _pServer = BLEDevice::createServer();
    BLEService *pService = _pServer->createService(SERVICE_UUID);
    _pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    _callbacks = new CredentialsCallbacks(this);
    _pCharacteristic->setCallbacks(_callbacks);
    pService->start();
    _pServer->getAdvertising()->start();

    DEBUG_PRINTLN("Waiting for a client connection to notify...");

    _bleStarted = true;
}

void BleManager::stopBLE()
{
    if (!_bleStarted)
        return;

    if (_pServer != nullptr)
    {
        _pServer->getAdvertising()->stop();
    }

    BLEDevice::deinit(false);

    delete _callbacks;
    _callbacks = nullptr;
    _pServer = nullptr;
    _bleStarted = false;
}

void BleManager::setWiFi(WiFiManager *wifiManager)
{
    _wifiManager = wifiManager;
}

void BleManager::beginTask()
{
    if (_bleTaskHandle != nullptr)
        return;

    xTaskCreate(
        startBleTask,
        "BLE Task",
        4096,
        this,
        1,
        &_bleTaskHandle);
}

bool BleManager::isBleStarted()
{
    return _bleStarted;
}