#pragma once

#include <NimBLEDevice.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...) 
#endif

// Calback for the WebBLe server
class MyServerCallback: public NimBLEServerCallbacks {
public:
    static bool deviceConnected;
    NimBLEServer *pServer;

    MyServerCallback(NimBLEServer *pServer);

    void onAuthenticationComplete(ble_gap_conn_desc* desc);

    void onDisconnect(BLEServer* pServer);
};

// BLE Server for Service handling
class MyBLEServer {
private:
    static NimBLEServer *pServer;
    static MyServerCallback *myServerCallback;
    static NimBLEAdvertising *pAdvertising;
    static bool initialized;

public:
    static void init(const char *deviceName = "ESP32");
    static void start();
    static NimBLEServer* getServer();
    static NimBLEAdvertising* getAdvertising();
};
