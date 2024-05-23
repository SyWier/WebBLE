#include <UniBLEServer.h>

// BLE Server for Service handling
UniBLEServer* UniBLEServer::uniBLEServer = nullptr;
NimBLEServer* UniBLEServer::pServer = nullptr;
NimBLEAdvertising* UniBLEServer::pAdvertising = nullptr;
uint32_t UniBLEServer::pinCode = 0;
bool UniBLEServer::isConnected = false;
bool UniBLEServer::isAuthenticated = false;
bool UniBLEServer::isInitialized = false;

void UniBLEServer::init(const char *deviceName /* "ESP32" */, uint32_t pinCode /* 0 */) {
    if(isInitialized) {
        DEBUG_MSG("BLEServer: Already initialized!\n");
        return;
    }

    DEBUG_MSG("Initializing BLEServer...\n");

    // Create singleton object for callbacks
    uniBLEServer = new UniBLEServer();

    // Init NimBLE on device
    NimBLEDevice::init(deviceName);

    // Set Maximum Transmission Unit (MTU)
    NimBLEDevice::setMTU(BLE_ATT_MTU_MAX);

    // Enable security
    if(pinCode == 0) {
        pinCode = random(000000, 999999); // PIN between [100.000, 999.999]
    }
    DEBUG_MSG("Enable security, PIN: %06d\n", pinCode);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(pinCode);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    // Create NimBLE server
    DEBUG_MSG("Creating Server...\n");
    pServer = NimBLEDevice::createServer();

    // Set callback class
    pServer->setCallbacks(uniBLEServer);

    // Get advertising handle
    pAdvertising = NimBLEDevice::getAdvertising();

    // Flag initialized state
    isInitialized = true;
}

void UniBLEServer::start() {
    // Start Advertising
    DEBUG_MSG("BLEServer: Start advertising!\n");
    pAdvertising->start();
}

NimBLEService* UniBLEServer::createService(const char* uuid) {
    return pServer->createService(uuid);
}

void UniBLEServer::adverticeService(const char* uuid) {
    pAdvertising->addServiceUUID(uuid);
}

// Calback for the WebBLe server
void UniBLEServer::onConnect(NimBLEServer* pServer) {
    UniBLEServer::isConnected = true;
    DEBUG_MSG("Device Connected\n");
    DEBUG_MSG("MTU Size: %d\n", NimBLEDevice::getMTU());
}

void UniBLEServer::onDisconnect(NimBLEServer* pServer) {
    UniBLEServer::isConnected = false;
    UniBLEServer::isAuthenticated = false;
    DEBUG_MSG("Device disconnected.\n");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    DEBUG_MSG("Start advertising\n");
}

void UniBLEServer::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    UniBLEServer::isAuthenticated = true;
    DEBUG_MSG("Authentication completed\n");
};