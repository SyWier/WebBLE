#include <NimBLEDevice.h>
// #include "EzBLE.h"
#include "WebCallback.h"
//#include "../EzBLE/EzBLE.h"

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

// WebBLE server
class WebServer {
private:
    NimBLEServer *pServer;
    NimBLEService *pService;
    NimBLECharacteristic *pSensorCharacteristic;
    NimBLECharacteristic *pLedCharacteristic;
    NimBLEAdvertising *pAdvertising;

    WebServerCallbacks *webServerCallbacks;
    WebCharacteristicCallbacks *webCharacteristicCallbacks;

public:
    WebServer() {
        pServer = nullptr;
        pService = nullptr;
        pSensorCharacteristic = nullptr;
        pLedCharacteristic = nullptr;
        pAdvertising = nullptr;

        webServerCallbacks = new WebServerCallbacks;
        webCharacteristicCallbacks = new WebCharacteristicCallbacks(2);
    }

    void Init() {
        Serial.println("Initializing WebBLE...");

        // Init NimBLE on device
        NimBLEDevice::init("ESP32");

        // Create NimBLE server
        Serial.println("Creating Server...");
        pServer = NimBLEDevice::createServer();
        // Set callback class
        pServer->setCallbacks(webServerCallbacks);

        // Create service with uuid
        Serial.println("Creating Service...");
        pService = pServer->createService(SERVICE_UUID);

        // Create sensor characteristic with uuid
        Serial.println("Creating Characteristics...");
        pSensorCharacteristic = pService->createCharacteristic(
            SENSOR_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::READ   |
            NIMBLE_PROPERTY::WRITE  |
            NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::INDICATE
        );

        // Create led characteristic with uuid
        // Default properties are being used (READ | WRITE)
        pLedCharacteristic = pService->createCharacteristic(
            LED_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::WRITE
        );
        // Set callback class
        pLedCharacteristic->setCallbacks(webCharacteristicCallbacks);
        
        // Start service
        Serial.println("Start service!");
        pService->start();

        // Get advertising handle
        pAdvertising = NimBLEDevice::getAdvertising();
        // Advertice service
        pAdvertising->addServiceUUID(SERVICE_UUID);
        //pAdvertising->setScanResponse(false);
        //pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
        pAdvertising->start();

        Serial.println("Waiting a client connection to notify...");
    }
    void Update() {
        Serial.println("(update)");
        static int value = 0;
        static bool oldDeviceConnected = false;
        bool deviceConnected = webServerCallbacks->deviceConnected;

        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            oldDeviceConnected = deviceConnected;
            Serial.println("Device disconnected.");
            delay(500); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            Serial.println("Start advertising");
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
            Serial.println("Device Connected");
        }
        // notify changed value
        if (deviceConnected) {
            value++;
            Serial.print("New value notified: ");
            Serial.println(value);
            pSensorCharacteristic->setValue(String(value).c_str());
            pSensorCharacteristic->notify();
            delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
        }
    }
    
};
