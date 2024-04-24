#include <Arduino.h>
#include <MyBLEServer.h>
#include <UniCom.h>
#include <RNTService.h>
#include <MyPasswordManager.h>

const int ledPin = 4;

UniCom uniCom;
MyPasswordManager myPasswordManager(uniCom);
RNTService rntService;


void setup() {
    Serial.begin(115200);

    MyBLEServer::init();

    myPasswordManager.init();
    rntService.init(ledPin);

    MyBLEServer::start();
}

void loop() {
    rntService.update();
    delay(1000);
}
