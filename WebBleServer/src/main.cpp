#include <Arduino.h>
//#include <EzBLE.h>
#include <Web.h>

WebServer webServer;

const int buttonPin = 10;

void setup() {
    Serial.begin(115200);
    delay(7000);

    // EzBLE EzBLE;
    // EzBLE.Init();
    // EzBLE.SetPacketValue("Hello BLE");
    // EzBLE.Start();

    webServer.Init();

    pinMode(buttonPin, INPUT);
}

void loop() {
    // int val = digitalRead(buttonPin);
    // String str = "Button state: ";
    // Serial.println(str + val);
    webServer.Update();
    delay(1000);
}
