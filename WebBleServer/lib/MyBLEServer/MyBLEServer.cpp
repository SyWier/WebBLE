#include <MyBLEServer.h>

// Calback for the WebBLe server

void MyServerCallback::onConnect(NimBLEServer* pServer) {
    MyBLEServer::isConnected = true;
    DEBUG_MSG("Device Connected\n");
}

void MyServerCallback::onDisconnect(NimBLEServer* pServer) {
    MyBLEServer::isConnected = false;
    MyBLEServer::isAuthenticated = false;
    DEBUG_MSG("Device disconnected.\n");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    DEBUG_MSG("Start advertising\n");
}

void MyServerCallback::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    MyBLEServer::isAuthenticated = true;
    DEBUG_MSG("Authentication completed\n");
};

// BLE Server for Service handling
NimBLEServer* MyBLEServer::pServer = nullptr;
NimBLEAdvertising* MyBLEServer::pAdvertising = nullptr;
uint32_t MyBLEServer::pinCode = 0;
bool MyBLEServer::isConnected = false;
bool MyBLEServer::isAuthenticated = false;
bool MyBLEServer::initialized = false;

void MyBLEServer::init(const char *deviceName /*"ESP32"*/) {
    if(initialized) {
        DEBUG_MSG("MyBLEServer: Already initialized!\n");
        return;
    }
    initialized = true;

    DEBUG_MSG("Initializing MyBLEServer...\n");

    // Init NimBLE on device
    NimBLEDevice::init(deviceName);

    // Enable security
    pinCode = random(000000, 999999); // PIN between [100.000, 999.999]
    DEBUG_MSG("Enable security, PIN: %06d\n", pinCode);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(pinCode);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    // Create NimBLE server
    DEBUG_MSG("Creating Server...\n");
    pServer = NimBLEDevice::createServer();

    // Set callback class
    pServer->setCallbacks(new MyServerCallback());

    // Get advertising handle
    pAdvertising = NimBLEDevice::getAdvertising();
}

void MyBLEServer::start() {
    // Start Advertising
    DEBUG_MSG("MyBLEServer: Start advertising!\n");
    pAdvertising->start();
}

NimBLEService* MyBLEServer::createService(const char* uuid) {
    return pServer->createService(uuid);
}

void MyBLEServer::adverticeService(const char* uuid) {
    pAdvertising->addServiceUUID(uuid);
}

// bool MyBLEServer::getPinCode() { return pinCode; }
// bool MyBLEServer::isConnected() { return deviceConnected; }
// NimBLEServer* MyBLEServer::getServer() { return pServer; }
// NimBLEAdvertising* MyBLEServer::getAdvertising() { return pAdvertising; }
// uint32_t MyBLEServer::getPinCode() { return pinCode; }
