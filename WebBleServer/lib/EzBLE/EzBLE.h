#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
//#include <NimBLEHIDDevice.h>
//#include <NimBLE2904.h>

//#include "../WebServer/Web.h"

class EzBLE {
private:
    NimBLEServer *pServer;
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;
    NimBLEAdvertising *pAdvertising;

public:
    EzBLE();
    void Init(const char *deviceName, const char *serviceUUID, const char *characteristicUUID);
    void SetPacketValue(String str);
    void InitWebServer();
    void Start();
};
