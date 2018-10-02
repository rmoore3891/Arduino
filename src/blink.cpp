#include <Arduino.h>

void setup() {
    pinMode(12,OUTPUT); Serial.begin(9600);
}

void loop() {
    Serial.println("Hi");
    digitalWrite(12,HIGH);
    delay(1000);
    digitalWrite(13,LOW); delay(1000);
    //Testing GIT Repository
}