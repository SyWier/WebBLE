#include <UniCom.h>

// Unicom Callback
void UniComCallback::onWrite(NimBLECharacteristic* pUniCharacteristic) {
    std::string value = pUniCharacteristic->getValue();
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

    switch(msgType) {
        case 1: 
            DEBUG_MSG("Sent value: Button A\n");
            pUniCharacteristic->setValue("Button A");
            break;
        case 2:
            DEBUG_MSG("Sent value: Button B\n");
            pUniCharacteristic->setValue("Button B");
            break;
        case 3:
            DEBUG_MSG("Sent value: Button C\n");
            pUniCharacteristic->setValue("Button C");
            break;
        default:
            DEBUG_MSG("Unknown button\n");
            return;
    }

    pUniCharacteristic->indicate();
}

// BLE Universal communication
void UniCom::init() {
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
    pCharacteristic->setCallbacks(new UniComCallback());

    // Start service
    DEBUG_MSG("Start UniCom service!\n");
    pService->start();

    // Advertice service
    MyBLEServer::adverticeService(UNI_SERVICE_UUID);
}
