// Basic demo for readings from Adafruit BNO08x WIP

#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "MS5837.h"
#include "TCA9548A.h"
#include <Adafruit_BNO08x.h>


void setup(void) {
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin();
  Serial.begin(115200);
  
 
}

void loop() {
  
}
