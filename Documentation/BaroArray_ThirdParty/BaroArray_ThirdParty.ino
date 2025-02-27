

/* Blue Robotics MS5837 Library Example
-----------------------------------------------------

Title: Blue Robotics MS5837 Library Example

Description: This example demonstrates the MS5837 Library with a connected
sensor. The example reads the sensor and prints the resulting values
to the serial terminal.

The code is designed for the Arduino Uno board and can be compiled and
uploaded via the Arduino 1.0+ software.

-------------------------------
The MIT License (MIT)

Copyright (c) 2015 Blue Robotics Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-------------------------------*/

#include <Wire.h>
#include "MS5837.h"
#include "TCA9548A.h"
#include <SoftwareSerial.h>

MS5837 sensor0;
MS5837 sensor1;
MS5837 sensor2;
MS5837 sensor3;
MS5837 sensor4;
MS5837 sensor5;
MS5837 sensor6;
MS5837 sensor7;

MS5837 sensor_arr[8] = {sensor0, sensor1, sensor2, sensor3, sensor4, sensor5, sensor6, sensor7};
int pressures[8] = {};

TCA9548A I2CMux;

void setup() {

  I2CMux.begin(Wire);             // Wire instance is passed to the library
  I2CMux.closeAll();  

  Serial.begin(9600);

  Serial.println("Starting");

  Wire.begin();

  for(int i = 0; i < 8; i++){
    I2CMux.openChannel(i);

    // Initialize pressure sensor
    // Returns true if initialization was successful
    // We can't continue with the rest of the program unless we can initialize the sensor
    while (!sensor_arr[i].init()) {
      Serial.println("Init failed!");
      Serial.println("Are SDA/SCL connected correctly?");
      Serial.println("\n\n\n");
      delay(1000);
    }

    // .init sets the sensor model for us but we can override it if required.
    // Uncomment the next line to force the sensor model to the MS5837_30BA.
    sensor_arr[i].setModel(MS5837::MS5837_30BA);
    sensor_arr[i].setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
    I2CMux.closeChannel(i);
  }
  
}

void loop() {
  // Update pressure and temperature readings
  
  Serial.println("Pressures (mbar): ");

  for(int i = 0; i < 8; i++){
    I2CMux.openChannel(i);
    sensor_arr[i].read();

    pressures[i] = sensor_arr[i].pressure();

    //Serial.print(sensor_arr[i].pressure());
    //Serial.print(" ");

    I2CMux.closeChannel(i);

    delay(150);
  }

  for(int i = 0; i < 8; i++){
    if(pressures[i] > 0){
      Serial.print(pressures[i]);
      Serial.print(" ");
    }
  }

  Serial.println(" ");
  
  /*Serial.print("Temperature: ");
  Serial.print(sensor.temperature());
  Serial.println(" deg C");

  Serial.print("Depth: ");
  Serial.print(sensor.depth());
  Serial.println(" m");

  Serial.print("Altitude: ");
  Serial.print(sensor.altitude());
  Serial.println(" m above mean sea level");*/

  delay(500);
}
