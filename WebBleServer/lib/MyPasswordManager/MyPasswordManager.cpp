#include <MyPasswordManager.h>

// Unicom Callback
MyPasswordManager::MyPasswordManager(UniCom* uniCom) : uniCom(uniCom) {}

void MyPasswordManager::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if(value.length() <= 0) {
        DEBUG_MSG("Invalid message received. (Insufficient lenght.)\n");
        return;
    }
    if(!MyBLEServer::isAuthenticated) {
        DEBUG_MSG("Device is not authenticated!\n");
        return;
    }

    int msgType = static_cast<int>(value[0]);

    DEBUG_MSG("Message received: ");
    DEBUG_MSG(String(msgType).c_str());
    DEBUG_MSG("\n");

    String val;

    switch(msgType) {
        case 1: 
            val = "Button A";
            DEBUG_MSG("Sent value: %s\n", val.c_str());
            pCharacteristic->indicate(val);
            break;
        case 2:
            DEBUG_MSG("Sending user info...\n");
            sendUserInfo();
            break;
        case 3:
            DEBUG_MSG("Sending password...\n");
            sendPassword();
            break;
        default:
            DEBUG_MSG("Unknown button\n");
            return;
    }
}

void MyPasswordManager::sendPassword() {
    // Allocate the JSON document
    JsonDocument json;

    // Add values in the document
    json["username"] = "Pelda Peter";
    json["password"] = "Pa55w0rd";

    uniCom->sendJSON(json);
}

void MyPasswordManager::sendUserInfo() {
    // Allocate the JSON document
    JsonDocument json;

    // Add values in the document
    json["username"] = "Minta Mano";
    json["email"] = "menomano@mmm.com";
    json["website"] = "mmm.banya.com";

    uniCom->sendJSON(json);
}
