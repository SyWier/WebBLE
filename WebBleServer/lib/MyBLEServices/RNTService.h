#include <NimBLEDevice.h>
#include <MyBLEServer.h>

// RandomNerdTutorial service
#define RNT_SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define RNT_SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define RNT_LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

// Calback for the WebBLe server
class LedCallback : public NimBLECharacteristicCallbacks {
public:
    int ledPin;

    LedCallback(int ledPin) {
        this->ledPin = ledPin;
        pinMode(ledPin, OUTPUT);
    };

    void onWrite(BLECharacteristic* pLedCharacteristic) {
        NimBLEAttValue value = pLedCharacteristic->getValue();

        if (value.length() > 0) {
            DEBUG_MSG("LED Characteristic event, written: %d\n", value.data()[0]);

            if (value.data()[0] == 1) {
                #ifdef ESP32_S3_DEVKITC
                neopixelWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);
                #else
                digitalWrite(ledPin, HIGH);
                #endif
            } else {
                #ifdef ESP32_S3_DEVKITC
                neopixelWrite(RGB_BUILTIN, 0, 0, 0);
                #else
                digitalWrite(ledPin, LOW);
                #endif
            }
        }
    }
};

// RandomNerdTutorial service
class RNTService {
private:
    NimBLEService *pService;
    NimBLECharacteristic *pSensorCharacteristic;
    NimBLECharacteristic *pLedCharacteristic;

public:
    void init(int ledPin) {
        // Create service with uuid
        DEBUG_MSG("Creating RNT Service...\n");
        pService =  MyBLEServer::createService(RNT_SERVICE_UUID);

        // Create sensor characteristic with uuid
        DEBUG_MSG("Creating RNT Fetch Characteristics...\n");
        pSensorCharacteristic = pService->createCharacteristic(
            RNT_SENSOR_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY
        );

        // Create led characteristic with uuid
        // Default properties are being used (READ | WRITE)
        DEBUG_MSG("Creating RNT LED Characteristics...\n");
        pLedCharacteristic = pService->createCharacteristic(
            RNT_LED_CHARACTERISTIC_UUID
        );

        // Set callback class
        pLedCharacteristic->setCallbacks(new LedCallback(ledPin));

        // Start service
        DEBUG_MSG("Start RNT service!\n");
        pService->start();

        // Advertice service
        MyBLEServer::adverticeService(RNT_SERVICE_UUID);
    }
    
    void update() {
        static int value = 0;

        // notify changed value
        if (MyBLEServer::isConnected) {
            String str = String(value++);
            DEBUG_MSG("New value notified: %s\n", str.c_str());
            pSensorCharacteristic->notify((const uint8_t*)str.c_str(), str.length());
        }
    }
};
