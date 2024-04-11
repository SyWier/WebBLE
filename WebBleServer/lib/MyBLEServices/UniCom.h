#pragma once

#include <MyBLEServer.h>
#include <ArduinoJson.h>

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// Callback function for user interface
class UniComCallback {
public:
    virtual void readValue(String value);
};

// UniCom callbacks called by Nimble
class UniComNimbleCallback : public NimBLECharacteristicCallbacks {
private:
    UniComCallback* callback;

public:
    void addCallback(UniComCallback* uniComCallback);
    void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code);
    void onWrite(NimBLECharacteristic* pCharacteristic);
};

// Universal Communication
class UniCom {
private:
    UniComNimbleCallback *uniComNimbleCallback;
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;

public:
    UniCom(UniComCallback* uniComCallback = nullptr);
    void init(UniComCallback* uniComCallback = nullptr);
    void sendString(String &str);
    void sendJSON(JsonDocument &json);
};