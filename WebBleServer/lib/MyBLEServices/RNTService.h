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
        std::string value = pLedCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("LED Characteristic event, written: ");
            Serial.println(static_cast<int>(value[0])); // Print the integer value

            int receivedValue = static_cast<int>(value[0]);
            if (receivedValue == 1) {
                digitalWrite(ledPin, HIGH);
            } else {
                digitalWrite(ledPin, LOW);
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
    LedCallback *ledCallback;

public:
    void init(int ledPin) {
        // Create service with uuid
        Serial.println("Creating RNT Service...");
        pService =  MyBLEServer::getServer()->createService(RNT_SERVICE_UUID);

        // Create sensor characteristic with uuid
        Serial.println("Creating RNT Fetch Characteristics...");
        pSensorCharacteristic = pService->createCharacteristic(
            RNT_SENSOR_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN |
            NIMBLE_PROPERTY::NOTIFY
        );

        // Create led characteristic with uuid
        // Default properties are being used (READ | WRITE)
        Serial.println("Creating RNT LED Characteristics...");
        pLedCharacteristic = pService->createCharacteristic(
            RNT_LED_CHARACTERISTIC_UUID,
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_ENC |
            NIMBLE_PROPERTY::WRITE_AUTHEN
        );

        // Set callback class
        ledCallback = new LedCallback(ledPin);
        pLedCharacteristic->setCallbacks(ledCallback);

        // Start service
        Serial.println("Start RNT service!");
        pService->start();

        // Advertice service
        MyBLEServer::getAdvertising()->addServiceUUID(RNT_SERVICE_UUID);
    }
    
    void update() {
        Serial.println("(update)");
        static int value = 0;

        // notify changed value
        if (MyServerCallback::deviceConnected) {
            String str = String(value++);
            Serial.print("New value notified: ");
            Serial.println(value);
            pSensorCharacteristic->notify((const uint8_t*)str.c_str(), str.length());
        }
    }
};
