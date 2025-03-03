#include "nfc.h"

uint8_t PN532_CMD_ACK[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
uint8_t PN532_CMD_FIRMWAREVER[] = {0x00, 0x00, 0xFF, 0x06, 0xFA, 0xD5};


void PN532::wakeup(){
    Wire.begin();
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    Wire.write(0x00); // write dummy frame
    Wire.endTransmission();
    /*NOTE: probably needs to be changed, adjusted on t_osc_start for when
    PN532 releases SCL. This value can be measured by checking how long
    SCL is held low after the wakeup command is sent

    Could optionally read SCL until it goes high
    */
    delay(2); 
    Wire.requestFrom(PN532_I2C_ADDRESS, 1);
    if(Wire.available()){
        byte response = Wire.read();
        #ifdef PN532_DEBUG
         Serial.print("PN532 Wake-up Response - 0x");
         Serial.println(response,HEX);
        #endif PN532_DEBUG
    } else {
        #ifdef PN532_DEBUG
         Serial.println("Error - PN532 did not response.");
        #endif PN532_DEBUG
    }
}

// restart, then wake up and start communication
bool PN532::begin(){
 
 return 0;
};

PN532::PN532(uint8_t IRQ_, uint8_t RST_) :
IRQ(IRQ_), RST(RST_)
{
pinMode(IRQ_,INPUT);
pinMode(RST_,OUTPUT);
Wire.beginTransmission(0x48);
}