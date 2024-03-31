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
    void onConnect(NimBLEServer* pServer);
    void onDisconnect(NimBLEServer* pServer);
    void onAuthenticationComplete(ble_gap_conn_desc* desc);
};

// BLE Server for Service handling
class MyBLEServer {
private:
    static NimBLEServer *pServer;
    static NimBLEAdvertising *pAdvertising;
    static bool initialized;

public:
    static uint32_t pinCode;
    static bool isConnected;
    static bool isAuthenticated;

public:
    static void init(const char *deviceName = "ESP32");
    static void start();
    static NimBLEService* createService(const char* uuid);
    static void adverticeService(const char* uuid);

    // static bool getPinCode();
    // static bool isConnected();
    // static NimBLEServer* getServer();
    // static NimBLEAdvertising* getAdvertising();
    // static uint32_t getPinCode();
};
