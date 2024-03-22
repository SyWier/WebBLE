#include <NimBLEDevice.h>
// #include "EzBLE.h"
#include "WebCallback.h"
//#include "../EzBLE/EzBLE.h"

// RandomNerdTutorial service
#define RNT_SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define RNT_SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define RNT_LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

// Universal communication service
#define UNI_SERVICE_UUID        "19b20000-e8f2-537e-4f6c-d104768a1214"
#define UNI_CHARACTERISTIC_UUID "19b20001-e8f2-537e-4f6c-d104768a1214"

// WebBLE server
class WebServer {
private:
    NimBLEServer *pServer;

    // RandomNerdTutorial service
    NimBLEService *pService;
    NimBLECharacteristic *pSensorCharacteristic;
    NimBLECharacteristic *pLedCharacteristic;
    ServerCallback *serverCallback;
    LedCallback *ledCallback;

    // Universal communication service
    NimBLEService *pUniService;
    NimBLECharacteristic *pUniCharacteristic;
    UniComCallback *uniComCallback;

    NimBLEAdvertising *pAdvertising;

    

public:
    WebServer() {
        pServer = nullptr;

        pService = nullptr;
        pSensorCharacteristic = nullptr;
        pLedCharacteristic = nullptr;
        serverCallback = new ServerCallback;
        ledCallback = new LedCallback(2);

        pUniService = nullptr;
        pUniCharacteristic = nullptr;
        uniComCallback = new UniComCallback;

        pAdvertising = nullptr;

        
    }

    void Init() {
        Serial.println("Initializing WebBLE...");

        // Init NimBLE on device
        NimBLEDevice::init("ESP32");

        // Create NimBLE server
        Serial.println("Creating Server...");
        pServer = NimBLEDevice::createServer();
        // Set callback class
        pServer->setCallbacks(serverCallback);

        // Get advertising handle
        pAdvertising = NimBLEDevice::getAdvertising();
        //pAdvertising->setScanResponse(false);
        //pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter

        // Init services
        InitRNTService();
        InitUniComService();
        
        // Start Advertising
        pAdvertising->start();

        Serial.println("Waiting a client connection to notify...");
    }

    // Init RandomNerdTutorial Service and Characteristics
    void InitRNTService() {
        // Create service with uuid
        Serial.println("Creating RNT Service...");
        pService = pServer->createService(RNT_SERVICE_UUID);

        // Create sensor characteristic with uuid
        Serial.println("Creating RNT Fetch Characteristics...");
        pSensorCharacteristic = pService->createCharacteristic(
            RNT_SENSOR_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::READ   |
            NIMBLE_PROPERTY::WRITE  |
            NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::INDICATE
        );

        // Create led characteristic with uuid
        // Default properties are being used (READ | WRITE)
        Serial.println("Creating RNT LED Characteristics...");
        pLedCharacteristic = pService->createCharacteristic(
            RNT_LED_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::WRITE
        );

        // Set callback class
        pLedCharacteristic->setCallbacks(ledCallback);
        
        // Start service
        Serial.println("Start RNT service!");
        pService->start();

        // Advertice service
        pAdvertising->addServiceUUID(RNT_SERVICE_UUID);
    }

    // Init Universal Communication Service and Characteristics
    void InitUniComService() {
        // Create service with uuid
        Serial.println("Creating UniCom Service...");
        pService = pServer->createService(UNI_SERVICE_UUID);

        // Create sensor characteristic with uuid
        Serial.println("Creating UniCom Characteristics...");
        pSensorCharacteristic = pService->createCharacteristic(
            UNI_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::READ   |
            NIMBLE_PROPERTY::WRITE  |
            NIMBLE_PROPERTY::NOTIFY |
            NIMBLE_PROPERTY::INDICATE
        );

        // Set callback class
        pLedCharacteristic->setCallbacks(uniComCallback);
        
        // Start service
        Serial.println("Start UniCom service!");
        pService->start();

        // Advertice service
        pAdvertising->addServiceUUID(UNI_SERVICE_UUID);
    }

    void Update() {
        Serial.println("(update)");
        static int value = 0;
        static bool oldDeviceConnected = false;
        bool deviceConnected = serverCallback->deviceConnected;

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
