#pragma once

#include <MyBLEServer.h>
#include <ArduinoJson.h>

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// Callback function for user interface
class UniComCallback {
public:
    virtual void readValue(String &value);
};

// Universal Communication
class UniCom : public NimBLECharacteristicCallbacks{
private:
    enum {
            ATT_HEADER = 3, // ATT header size for write, read, notification, indication
            UNICOM_HEADER = 1,
    };
    int att_mtu;
    int att_data;
    int packet_data;
    int packet_size;

    String inBuffer;
    String outBuffer;
    int str_pos;
    bool isInProgress;

private:
    std::function<void(String &str)> callback;
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;

public:
    UniCom(int bufferSize = 2000);
    void init();
    void addCallback(std::function<void(String &str)> callback);
    
    void sendPacket();
    void sendString(String &str);
    void sendJSON(JsonDocument &json);
    
    // Nimble callback functions
    void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code);
    void onWrite(NimBLECharacteristic* pCharacteristic);
};
