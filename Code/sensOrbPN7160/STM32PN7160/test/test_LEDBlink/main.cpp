#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

void setup(){
    Serial.begin(115200);
 
    Wire.setSDA(PB7);
    Wire.setSCL(PB6);
    Wire.begin();
       
    pinMode(PA13, OUTPUT);
    pinMode(PB15, OUTPUT);
}

void loop(){
    Serial.println("Apple");
    digitalWrite(PA13, HIGH);
    digitalWrite(PB15, LOW);
    delay(1000);
    digitalWrite(PA13, LOW);
    digitalWrite(PB15, HIGH);
    delay(1000);
}
