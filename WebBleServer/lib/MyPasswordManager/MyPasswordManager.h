#include <UniCom.h>

// Calback for the UniCom Characteristics
class MyPasswordManager {
private:
    UniCom &uniCom;

public:
    MyPasswordManager(UniCom &uniCom) : uniCom(uniCom) {}
    void init();
    void getPacket(UniCom::Packet packet);
    void typeDecoder(int type);
    void sendPassword();
    void sendUserInfo();
};
