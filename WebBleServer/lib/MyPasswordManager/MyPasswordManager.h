#include <UniCom.h>

// Calback for the UniCom Characteristics
class MyPasswordManager : public NimBLECharacteristicCallbacks {
private:
    UniCom* uniCom;

public:
    MyPasswordManager(UniCom* uniCom);
    void onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code);
    void onWrite(NimBLECharacteristic* pCharacteristic);
    void sendPassword();
    void sendUserInfo();
};
