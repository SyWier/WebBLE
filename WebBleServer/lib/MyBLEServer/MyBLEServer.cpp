#include <MyBLEServer.h>

// Calback for the WebBLe server
bool MyServerCallback::deviceConnected = false;

MyServerCallback::MyServerCallback(NimBLEServer *pServer) : pServer(pServer) {};

void MyServerCallback::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    deviceConnected = true;
    Serial.println("Device Connected");
};

void MyServerCallback::onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
}

// BLE Server for Service handling
NimBLEServer* MyBLEServer::pServer = nullptr;
MyServerCallback* MyBLEServer::myServerCallback = nullptr;
NimBLEAdvertising* MyBLEServer::pAdvertising = nullptr;
bool MyBLEServer::initialized = false;

void MyBLEServer::init(const char *deviceName /*"ESP32"*/) {
    if(initialized) {
        Serial.println("MyBLEServer: Already initialized!");
        return;
    }
    initialized = true;

    Serial.println("Initializing MyBLEServer...");

    // Init NimBLE on device
    NimBLEDevice::init(deviceName);

    // Enable security
    Serial.println("Enable security, PIN: 123456");
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(123456);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    // Create NimBLE server
    Serial.println("Creating Server...");
    pServer = NimBLEDevice::createServer();

    // Set callback class
    myServerCallback = new MyServerCallback(pServer);
    pServer->setCallbacks(myServerCallback);

    // Get advertising handle
    pAdvertising = NimBLEDevice::getAdvertising();
}

void MyBLEServer::start() {
    // Start Advertising
    Serial.println("MyBLEServer: Start advertising!");
    pAdvertising->start();
}

NimBLEServer* MyBLEServer::getServer() { return pServer; }
NimBLEAdvertising* MyBLEServer::getAdvertising() { return pAdvertising; }
