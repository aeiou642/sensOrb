#include <Arduino.h>
#include <Wire.h>
#include "PN532.h"
#include "SparkFun_BNO08x_Arduino_Library.h"

PN532 pn_rx;
BNO08x imu;
#define PN532_BUF_SIZE 60

enum ReceiverState {
  STATE_INIT,
  STATE_WAIT,
  STATE_EXCHANGE,
  STATE_SEND
};

ReceiverState currentState = STATE_INIT;
char sensorResponse[128];
uint8_t rxBuffer[PN532_BUF_SIZE];


void setup() {
  Serial.begin(115200);
  delay(100);
  pn_rx.begin();
  imu.begin();
  Wire.setClock(400000);
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
        
        break;
    }
    case STATE_SEND:
    {
        break;
    }
}
}