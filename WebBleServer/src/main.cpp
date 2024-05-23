#include <Arduino.h>
#include <UniBLEServer.h>
#include <UniCom.h>
#include <RNTService.h>
#include <MyPasswordManager.h>

const int ledPin = 4;

UniCom uniCom;
MyPasswordManager myPasswordManager(uniCom);
RNTService rntService;


void setup() {
    Serial.begin(115200);

    UniBLEServer::init();

    myPasswordManager.init();
    rntService.init(ledPin);

    UniBLEServer::start();
}

void loop() {
    rntService.update();
    delay(1000);
}
