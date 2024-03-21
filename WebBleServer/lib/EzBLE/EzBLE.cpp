#include <EzBLE.h>

EzBLE::EzBLE() {
    pServer = nullptr;
    pService = nullptr;
    pCharacteristic = nullptr;
    pAdvertising = nullptr; 
}

void EzBLE::Init(const char *deviceName, const char *serviceUUID, const char *characteristicUUID) {
    // Init NimBLE on device
    NimBLEDevice::init(deviceName);
    // Create NimBLE server
    pServer = NimBLEDevice::createServer();
    // Create service with uuid
    pService = pServer->createService(serviceUUID);
    // Create characteristic with uuid
    // Default properties are being used (READ | WRITE)
    pCharacteristic = pService->createCharacteristic(characteristicUUID);
    // Get advertising handle
    pAdvertising = NimBLEDevice::getAdvertising();
}



void EzBLE::SetPacketValue(String str) {
    pCharacteristic->setValue(str);
}

void EzBLE::Start() {
    // Start service
    pService->start();
    // Advertice service
    pAdvertising->addServiceUUID("ABCD");
    pAdvertising->start();
}
