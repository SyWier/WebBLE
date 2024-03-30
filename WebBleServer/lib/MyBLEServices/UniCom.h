#include <NimBLEDevice.h>
#include <MyBLEServer.h>

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// Calback for the UniCom Characteristics
class UniComCallback : public NimBLECharacteristicCallbacks {
public:
    void onWrite(NimBLECharacteristic* pUniCharacteristic);
};

// BLE Universal communication
class UniCom {
private:
    NimBLEService *pService;
    NimBLECharacteristic *pCharacteristic;
    UniComCallback *uniComCallback;

public:
    void init();
};