#include <Arduino.h>
<<<<<<< HEAD
#include "PN532.h"
#include "SparkFun_BNO08x_Arduino_Library.h"
=======
#include <Wire.h>
#include "PN532.h"
#include

PN532 pn_rx;
Adafruit_BNO08x imu;
sh2_SensorValue_t sensorValue;

#define PN532_BUF_SIZE 60

enum ReceiverState {
  STATE_INIT,
  STATE_WAIT,
  STATE_EXCHANGE,
  STATE_SEND
};

ReceiverState currentState = STATE_INIT;


void setup() {
  Serial.begin(115200);
  delay(100);
  pn_rx.begin();
  if(!imu.begin_I2C()){
    Serial.println("Failed to find BNO08x");
    while(1){
      delay(10);
    }
  };
  Serial.println("Both chips found. Proceeding");
  Wire.setClock(400000);
}
void setReports(void){
  if (!imu.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!imu.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
    Serial.println("Could not enable gyroscope");
  }
}

void loop() {
    switch(currentState){
    case STATE_WAIT:
    {
       if(!pn_rx.IsReady()){
        Serial.println("Initializing NFC Target");
        currentState = STATE_INIT;
       }
        break;
    }
    case STATE_INIT:
    {
        uint8_t tgInitAsTarget[]{
            PN532_COMMAND_TGINITASTARGET, // Set as target
            0x01,             // Mode: Supports Passive 106 kbps (DEP only)
        
            // No MiFareParams (skip 6 bytes)
        
            // FeliCaParams (18 bytes, optional, can be left zeroed)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        
            // NFCID3t (10 Bytes - Required for P2P)
            0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44,
        
            // General Bytes (Used for Parameter Exchange)
            0x46, 0x66, 0x6D, 0x01, 0x01, 0x10, 0x02, 0x02, 0x00, 0x00,
        
            // Checksum, Postamble
            0xC5, 0x00
        };
        if(!pn_rx.SendCommandCheckAck(tgInitAsTarget,sizeof(tgInitAsTarget))){
            Serial.println("tgInitAsTarget failed -- retrying");
            return;
        }
        Serial.println("NFC Target Ready");
        currentState = STATE_EXCHANGE;
        break;
    }
    case STATE_EXCHANGE:
    {
        uint8_t readBuffer[64];
        int16_t length = pn_rx.ReadData(readBuffer, sizeof(readBuffer));
        if (length > 0){
            Serial.print("Recieved: ");
            for(int i = 0; i < length; i++){
                // check to see if it's a valid ascii character
                if(readBuffer[i] >= 32 && readBuffer[i] < 127){
                    Serial.write(readBuffer[i]);
                }
                else {
                    Serial.print("0x");
                    Serial.print(readBuffer[i],HEX);
                    Serial.print(" ");
                }
            }
        }
        Serial.println("Sending data...");
        currentState = STATE_SEND;
        break;
    }
    case STATE_SEND:
    {
        float imuData[6]{0, 0, 0, 0, 0, 0};
        imuData[0] = sensorValue.un.accelerometer.x;
        imuData[1] = sensorValue.un.accelerometer.y;
        imuData[2] = sensorValue.un.accelerometer.z;
        imuData[3] = sensorValue.un.gyroscope.x;
        imuData[4] = sensorValue.un.gyroscope.y;
        imuData[5] = sensorValue.un.gyroscope.z;
        uint8_t imuDataBuffer[6];
        size_t numElements = sizeof(imuData) / sizeof(imuData[0]);
        for(size_t i = 0; i < numElements; i++){
            imuDataBuffer[i] = (uint8_t)constrain(roundf((imuData[i]) * 10),0,255);
        } // convert the data from float to uint8_t once received 
        uint8_t sensorData[] = {
            PN532_COMMAND_INDATAEXCHANGE,
            0x01, // target number
            imuDataBuffer[0], // ax
            imuDataBuffer[1], // ay
            imuDataBuffer[2], // az
            imuDataBuffer[3], // gx
            imuDataBuffer[4], // gy
            imuDataBuffer[5] // gz
        };
        if(!pn_rx.SendCommandCheckAck(sensorData,sizeof(sensorData))){
            Serial.println("inDataExchange failed, retrying");
            return;
        }
        Serial.println("Sensor data sent");
        delay(100);
        break;
    }
}
}
>>>>>>> 817e138a0db2410a293c0d598204cfabae639c73
