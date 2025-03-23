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
    }
    currentState = P2P_INJUMP;
    break;
    /*======================================================*/
    case P2P_INJUMP:
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
    /*======================================================*/
    case P2P_EXCHANGE:
    
    break;
    /*======================================================*/
    case P2P_RELEASE:
    break;
    /*======================================================*/
    case P2P_DONE:
    break;
    /*======================================================*/
  }
}