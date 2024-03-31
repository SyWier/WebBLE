#include <MyBLEServer.h>

// Calback for the WebBLe server
bool MyServerCallback::deviceConnected = false;

MyServerCallback::MyServerCallback(NimBLEServer *pServer) : pServer(pServer) {};

void MyServerCallback::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    deviceConnected = true;
    DEBUG_MSG("Device Connected\n");
};

void MyServerCallback::onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    DEBUG_MSG("Device disconnected.\n");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    DEBUG_MSG("Start advertising\n");
}

// BLE Server for Service handling
NimBLEServer* MyBLEServer::pServer = nullptr;
MyServerCallback* MyBLEServer::myServerCallback = nullptr;
NimBLEAdvertising* MyBLEServer::pAdvertising = nullptr;
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
    // Password between [100.000, 999.999] ([100.000, 1.000.000[)
    pinCode = random(000000, 999999);
    DEBUG_MSG("Enable security, PIN: %06d\n", pinCode);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(pinCode);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    // Create NimBLE server
    DEBUG_MSG("Creating Server...\n");
    pServer = NimBLEDevice::createServer();

    // Set callback class
    myServerCallback = new MyServerCallback(pServer);
    pServer->setCallbacks(myServerCallback);

    // Get advertising handle
    pAdvertising = NimBLEDevice::getAdvertising();
}

void MyBLEServer::start() {
    // Start Advertising
    DEBUG_MSG("MyBLEServer: Start advertising!\n");
    pAdvertising->start();
}

NimBLEServer* MyBLEServer::getServer() { return pServer; }
NimBLEAdvertising* MyBLEServer::getAdvertising() { return pAdvertising; }
uint32_t MyBLEServer::getPinCode() { return pinCode; }
