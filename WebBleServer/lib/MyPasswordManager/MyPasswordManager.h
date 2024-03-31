#include <UniCom.h>

// Calback for the UniCom Characteristics
class MyPasswordManager : public NimBLECharacteristicCallbacks {
private:
    UniCom* uniCom;

public:
    MyPasswordManager(UniCom* uniCom);
    void onWrite(NimBLECharacteristic* pCharacteristic);
    void sendPassword();
    void sendUserInfo();
};
