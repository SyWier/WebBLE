#pragma once

#include <NimBLEDevice.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...) 
#endif

// BLE Server for Service handling
class UniBLEServer: public NimBLEServerCallbacks {
private:
    static UniBLEServer* uniBLEServer; // "this" pointer, Singleton setup
    static NimBLEServer *pServer;
    static NimBLEAdvertising *pAdvertising;

public:
    static uint32_t pinCode;
    static bool isInitialized;
    static bool isConnected;
    static bool isAuthenticated;

public:
    static void init(const char *deviceName = "ESP32", uint32_t pinCode = 0);
    static void start();
    static NimBLEService* createService(const char* uuid);
    static void adverticeService(const char* uuid);

    // NimBLE callbacks
    void onConnect(NimBLEServer* pServer);
    void onDisconnect(NimBLEServer* pServer);
    void onAuthenticationComplete(ble_gap_conn_desc* desc);
};
