

#include <Wire.h>
#include "MS5837.h"
#include "TCA9548A.h"
#include <SoftwareSerial.h>
#include <Arduino.h>

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

  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
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
