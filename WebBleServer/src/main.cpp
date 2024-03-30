#include <Arduino.h>
#include <MyBLEServer.h>
#include <UniCom.h>
#include <RNTService.h>

// const int buttonPin = 10;
const int ledPin = 2;

UniCom uniCom;
RNTService rntService;

void setup() {
    Serial.begin(115200);
    // delay(7000);

    MyBLEServer::init();

    uniCom.init();
    rntService.init(ledPin),

    MyBLEServer::start();

    // pinMode(buttonPin, INPUT);
}

void loop() {
    // int val = digitalRead(buttonPin);
    // String str = "Button state: ";
    // Serial.println(str + val);
    rntService.update();
    delay(1000);
}
