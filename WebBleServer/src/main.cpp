#include <Arduino.h>
#include <MyBLEServer.h>
#include <UniCom.h>
#include <RNTService.h>
#include <MyPasswordManager.h>

// const int buttonPin = 10;
const int ledPin = 4;

MyPasswordManager myPasswordManager;
RNTService rntService;


void setup() {
    Serial.begin(115200);
    while (!Serial);
    // delay(7000);

    MyBLEServer::init();

    myPasswordManager.init();
    rntService.init(ledPin);

    MyBLEServer::start();

    // pinMode(buttonPin, INPUT);
}

void loop() {
    // int val = digitalRead(buttonPin);
    // String str = "Button state: ";
    // Serial.println(str + val);
    rntService.update();
    // DEBUG_MSG("MTU: %d\n", NimBLEDevice::getMTU());
    delay(1000);
}
