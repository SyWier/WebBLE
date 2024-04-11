#include <MyPasswordManager.h>

// Unicom Callback
MyPasswordManager::~MyPasswordManager() {
    delete uniCom;
}

void MyPasswordManager::init() {
    DEBUG_MSG("Initializing MyPasswordManager...\n");
    uniCom = new UniCom(this);
}

void MyPasswordManager::readValue(String value) {
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

    typeDecoder(msgType);
}

void MyPasswordManager::typeDecoder(int type) {
    String val;
    switch(type) {
        case 1: 
            val = "Button A";
            DEBUG_MSG("Sent value: %s\n", val.c_str());
            uniCom->sendString(val);
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
