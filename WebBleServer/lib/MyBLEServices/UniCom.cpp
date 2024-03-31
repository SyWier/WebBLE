#include <UniCom.h>

// BLE Universal communication
void UniCom::init(NimBLECharacteristicCallbacks* pCallbacks) {
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
    pCharacteristic->setCallbacks(pCallbacks);

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
