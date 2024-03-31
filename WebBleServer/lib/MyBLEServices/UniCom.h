#pragma once

#include <MyBLEServer.h>
#include <ArduinoJson.h>

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// BLE Universal communication
class UniCom {
private:
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;

public:
    void init(NimBLECharacteristicCallbacks* pCallbacks);
    void sendString(String &str);
    void sendJSON(JsonDocument &json);
};