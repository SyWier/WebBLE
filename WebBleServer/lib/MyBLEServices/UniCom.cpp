#include <UniCom.h>

// Unicom Callback
void UniComCallback::onWrite(NimBLECharacteristic* pUniCharacteristic) {
    std::string value = pUniCharacteristic->getValue();
    int msgType = static_cast<int>(value[0]);
    if(value.length() <= 0) {
        Serial.print("Invalid message received. (Insufficient lenght.)");
        return;
    }
    if(!MyServerCallback::deviceConnected) {
        return;
    }

    Serial.print("Message received: ");
    Serial.println(msgType); // Print the integer value

    switch(msgType) {
        case 1: 
            Serial.println("Button A");
            pUniCharacteristic->setValue("Button A");
            break;
        case 2:
            Serial.println("Button B");
            pUniCharacteristic->setValue("Button B");
            break;
        case 3:
            Serial.println("Button C");
            pUniCharacteristic->setValue("Button C");
            break;
        default:
            Serial.println("Unknown button");
            return;
    }

    pUniCharacteristic->notify();
}

// BLE Universal communication
void UniCom::init() {
    Serial.println("Creating UniCom Service...");
    pService = MyBLEServer::getServer()->createService(UNI_SERVICE_UUID);
    
    // Create UniCom Characteristic with UUID
    Serial.println("Creating UniCom Characteristics...");
    pCharacteristic = pService->createCharacteristic(
        UNI_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ   |
        NIMBLE_PROPERTY::WRITE  |
        NIMBLE_PROPERTY::NOTIFY |
        NIMBLE_PROPERTY::INDICATE
    );

    // Set callback class
    uniComCallback = new UniComCallback();
    pCharacteristic->setCallbacks(uniComCallback);

    // Start service
    Serial.println("Start UniCom service!");
    pService->start();

    // Advertice service
    MyBLEServer::getAdvertising()->addServiceUUID(UNI_SERVICE_UUID);
}
