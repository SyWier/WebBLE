#include <UniCom.h>

// Calback for the UniCom Characteristics
class MyPasswordManager : public UniComCallback {
private:
    UniCom &uniCom;

public:
    MyPasswordManager(UniCom &uniCom) : uniCom(uniCom) {}
    void init();
    void readValue(String &value);
    void typeDecoder(int type);
    void sendPassword();
    void sendUserInfo();
};