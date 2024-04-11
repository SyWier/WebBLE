#include <UniCom.h>

// BLE UniCom Callback functions
void UniComNimbleCallback::addCallback(UniComCallback* uniComCallback) {
    this->callback = uniComCallback;
}

void UniComNimbleCallback::onStatus(NimBLECharacteristic* pCharacteristic, Status s, int code) {
    String str;
    switch(s) {
        case SUCCESS_INDICATE: str = "SUCCESS_INDICATE"; break;
        case SUCCESS_NOTIFY: str = "SUCCESS_NOTIFY"; break;
        case ERROR_INDICATE_DISABLED: str = "ERROR_INDICATE_DISABLED"; break;
        case ERROR_NOTIFY_DISABLED: str = "ERROR_NOTIFY_DISABLED"; break;
        case ERROR_GATT: str = "ERROR_GATT"; break;
        case ERROR_NO_CLIENT: str = "ERROR_NO_CLIENT"; break;
        case ERROR_INDICATE_TIMEOUT: str = "ERROR_INDICATE_TIMEOUT"; break;
        case ERROR_INDICATE_FAILURE: str = "ERROR_INDICATE_FAILURE"; break;
        default: str = "Unkown status"; break;
    }
    DEBUG_MSG("Status: %s.\n", str.c_str());
}

void UniComNimbleCallback::onWrite(NimBLECharacteristic* pCharacteristic) {
    callback->readValue(pCharacteristic->getValue());
}

// BLE Universal communication
UniCom::UniCom(UniComCallback* uniComCallback /*nullptr*/) {
    init(uniComCallback);
}

void UniCom::init(UniComCallback* uniComCallback /*nullptr*/) {
    DEBUG_MSG("Initializing UniCom...\n");
    if(!MyBLEServer::isInitialized) {
        DEBUG_MSG("Error: MyBLEServer hasn't started yet!\n");
        return;
    }
    static bool initialized = false;
    if(initialized) {
        DEBUG_MSG("UniCom already initialized!\n");
        return;
    }
    initialized = true;

    DEBUG_MSG("Creating UniCom Service...\n");
    pService = MyBLEServer::createService(UNI_SERVICE_UUID);

    // Create UniCom Characteristic with UUID
    DEBUG_MSG("Creating UniCom Characteristics...\n");
    pCharacteristic = pService->createCharacteristic(
        UNI_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::READ_ENC |
        NIMBLE_PROPERTY::READ_AUTHEN |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::WRITE_ENC |
        NIMBLE_PROPERTY::WRITE_AUTHEN |
        NIMBLE_PROPERTY::INDICATE
    );

    // Set callback class
    uniComNimbleCallback = new UniComNimbleCallback();
    uniComNimbleCallback->addCallback(uniComCallback);
    pCharacteristic->setCallbacks(uniComNimbleCallback);

    // Start service
    DEBUG_MSG("Start UniCom service!\n");
    pService->start();

    // Advertice service
    MyBLEServer::adverticeService(UNI_SERVICE_UUID);
}

void UniCom::sendString(String &str) {
    DEBUG_MSG("Sending String...\n");
    pCharacteristic->indicate(str);
}

void UniCom::sendJSON(JsonDocument &json) {
    DEBUG_MSG("Sending JSON...\n");
    // Generate the minified JSON and send it
    String str;
    serializeJson(json, str);
    DEBUG_MSG("Serialized JSON: %s\n", str.c_str());
    sendString(str);
}
