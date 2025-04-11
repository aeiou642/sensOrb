#include <Arduino.h>
#include "PN532.h"
#include "SparkFun_BNO08x_Arduino_Library.h"

/*
Using a state machine implementation makes this a little bit easier
to debug on my end. Theoretically, I could simulate this in Quartus.
I'm not simulating it yet though. - OP

It also inheriently makes it so that if it fails, it will try again
next cycle for any given command.
*/
PN532 pn_tx;
enum P2PInitiator{
  P2P_INIT = 0,
  P2P_INJUMP,
  P2P_EXCHANGE,
  P2P_RELEASE,
  P2P_DONE
};

P2PInitiator currentState = P2P_INIT;
bool configSAM = false;

void setup(){
  Serial.begin(115200);
  pn_tx.begin();
<<<<<<< HEAD
=======
  Wire.setClock(400000);
>>>>>>> 817e138a0db2410a293c0d598204cfabae639c73
}
void loop(){
  switch(currentState){
    /*======================================================*/
    case P2P_INIT:
    {
    if(!configSAM){
      Serial.println("Configuring SAM");
      if(!pn_tx.SamConfig()){
        Serial.println("Failed SAM Configuration.. Retrying");
        return;
      }
      configSAM = true;
    }
<<<<<<< HEAD
    }
    currentState = P2P_INJUMP;
    break;
    /*======================================================*/
    case P2P_INJUMP:
=======
    currentState = P2P_INJUMP;
    break;
  }
    /*======================================================*/
    case P2P_INJUMP:
    {
>>>>>>> 817e138a0db2410a293c0d598204cfabae639c73
      Serial.println("Sending InJumpForDEP (Passive @ 106 kbps)");
      {
      uint8_t inJumpForDEP[]={
        PN532_COMMAND_INJUMPFORDEP, // InJumpForDEP
        0x02, // configure to passive mode
        0x00, // set to 106 kbps
        0x00  // .. with no general bytes
      };
      if(!pn_tx.SendCommandCheckAck(inJumpForDEP,sizeof(inJumpForDEP))){
        Serial.println("inJumpForDEP -- NACK -- Retrying...");
        return;
      }
      uint8_t readBuffer[60];
      int16_t length = pn_tx.ReadData(readBuffer,sizeof(readBuffer));
      if(length < 3 || readBuffer[1] != (inJumpForDEP[0] + 1)){
        Serial.println("inJumpForDEP -- Failed to find target -- Retrying...");
        return;
      }
      Serial.println("inJumpForDEP successful");
    }
    currentState = P2P_EXCHANGE;
    break;
<<<<<<< HEAD
    /*======================================================*/
    case P2P_EXCHANGE:
    
    break;
    /*======================================================*/
    case P2P_RELEASE:
    break;
=======
  }
    /*======================================================*/
    case P2P_EXCHANGE:
    {
    Serial.println("Beginning inDataExchange");
      uint8_t inDataExchangeCmd[]{
        PN532_COMMAND_INDATAEXCHANGE,
        0x01, // Target number (only one target, so ID is 1)
        // Test payload: Hello
        0x48,0x65,0x6C,0x6F
      };
      if(!pn_tx.SendCommandCheckAck(inDataExchangeCmd,sizeof(inDataExchangeCmd))){
        Serial.println("inDataExchange -- NACK -- Retrying...");
        return;
      }
      uint8_t readBuffer[60];
      int16_t length = pn_tx.ReadData(readBuffer,sizeof(readBuffer));
      if(length > 0){
        Serial.print("Recieved: ");
        for(int i = 0; i < length; i++){
          if(readBuffer[i] >= 32 && readBuffer[i] < 127){
            Serial.write(readBuffer[i]);
          } else {
            Serial.print("0x");
            Serial.print(readBuffer[i],HEX);
            Serial.print(" ");
          }
        }
        Serial.println();
      }
      Serial.println("inDataExchange successful");
    
    currentState = P2P_RELEASE;
    break;
    }
    /*======================================================*/
    case P2P_RELEASE:
    {
    uint8_t InRelease[]{
      PN532_COMMAND_INRELEASE, 
      0x01 // Target ID/number to release
    };
    if(!pn_tx.SendCommandCheckAck(InRelease,sizeof(InRelease))){
      Serial.println("inRelease -- NACK -- Retrying...");
      return;
    }
    Serial.println("Target released");
    break;
  }
>>>>>>> 817e138a0db2410a293c0d598204cfabae639c73
    /*======================================================*/
    case P2P_DONE:
    break;
    /*======================================================*/
  }
}