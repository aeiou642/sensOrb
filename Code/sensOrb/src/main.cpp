#include <Arduino.h>
#include "PN532.h"
#include "SparkFun_BNO08x_Arduino_Library.h"

/*

*/
PN532 pn_tx;
BNO08x imu;

uint8_t inReq[]{0x4A,0x01,0x00};
uint8_t sendMessage[] = {0x40,0x01,0x48,0x65,0x6C,0x6C,0x6F};
uint8_t readData[] = {0x86};

void setup() {
  Serial.begin(115000);
  Serial.println("Intializing PN532, BNO086");
  byte type,versionMSB,versionLSB,flags;
  pn_tx.begin();
  if(pn_tx.GetFirmwareVersion(&type,&versionMSB,&versionLSB,&flags)){
    Serial.print("PN532 Found! Firmware information:");
    Serial.print("IC Type: 0x"); Serial.print(type,HEX);
    Serial.print(", Version: "); Serial.print(versionMSB,DEC);
    Serial.print("."); Serial.println(versionLSB, DEC);
  } else{
    Serial.println("Failed to get PN532 Firmware Version");
  }
  imu.begin();
}

void loop() {
  // buffer to recieve commands
  byte readBuffer[256];
  /*
    InJumpForDEP (0x56) in passive mode (0x01), reading  at 106 kbps (0x00)
    with no general bytes (0x00). Check for target by checking for ACK bit
    Afterwards, read the response to confirm activation
  */
  uint8_t inJumpForDEP[] = {0x56, 0x01, 0x02, 0x00};
  if(!pn_tx.SendCommandCheckAck(inJumpForDEP, sizeof(inJumpForDEP))){
    Serial.println("P2P FAIL - NACK");
    while(true);
  };
  int16_t length = pn_tx.ReadData(readBuffer,sizeof(readBuffer));
  if(length < 3 || readBuffer[1] != (inJumpForDEP[0] + 1)){
    Serial.println("P2P FAIL - No Target");
    while(true);
  }
  Serial.println("P2P OK - DE");
  /*
    InDataExchange (0x40) sending to target 1 (0x01), sending "Test" byte
    by byte. Read exchanged data
  */
  uint8_t InDataExchange[] = {0x40, 0x01,'T','e','s','t'};
  if(!pn_tx.SendCommandCheckAck(InDataExchange, sizeof(InDataExchange))){
    Serial.println("DATA - NACK");
    while(true);
  };
  // exchage the data
  //pn_tx.SendCommandCheckAck();
  //InRelease
 // pn_tx.SendCommandCheckAck();
}

