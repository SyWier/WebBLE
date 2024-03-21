#include <Arduino.h>
#include <NimBLEDevice.h>

// Calback for the WebBLe server
class WebServerCallbacks: public NimBLEServerCallbacks {
public:
    bool deviceConnected;

    WebServerCallbacks() : deviceConnected(false) {};

    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// Calback for the WebBLe server
class WebCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
    int ledPin;

    WebCharacteristicCallbacks(int ledPin) {
        this->ledPin = ledPin;
        pinMode(ledPin, OUTPUT);
    };

    void onWrite(BLECharacteristic* pLedCharacteristic) {
        std::string value = pLedCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("Characteristic event, written: ");
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
