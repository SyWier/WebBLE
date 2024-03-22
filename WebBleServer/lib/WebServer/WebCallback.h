#include <Arduino.h>
#include <NimBLEDevice.h>

// Calback for the WebBLe server
class ServerCallback: public NimBLEServerCallbacks {
public:
    bool deviceConnected;

    ServerCallback() : deviceConnected(false) {};

    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

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

// Calback for the UniCom Characteristics
class UniComCallback : public NimBLECharacteristicCallbacks {
public:
    bool *pDeviceConnected;
    UniComCallback(bool *pDeviceConnected) {
        this->pDeviceConnected = pDeviceConnected;
    }

    void onWrite(NimBLECharacteristic* pUniCharacteristic) {
        std::string value = pUniCharacteristic->getValue();
        int msgType = static_cast<int>(value[0]);
        if(value.length() <= 0) {
            Serial.print("Invalid message received. (Insufficient lenght.)");
            return;
        }
        if(!*pDeviceConnected) {
            return;
        }

        Serial.print("Message received: ");
        Serial.println(msgType); // Print the integer value

        
        

        switch(msgType) {
            case 1: 
                Serial.println("Button A");
                pUniCharacteristic->setValue("Button A");
                break;
            case 2:
                Serial.println("Button B");
                pUniCharacteristic->setValue("Button B");
                break;
            case 3:
                Serial.println("Button C");
                pUniCharacteristic->setValue("Button C");
                break;
            default:
                Serial.println("Unknown button");
                return;
        }

        pUniCharacteristic->notify();
    }
};
