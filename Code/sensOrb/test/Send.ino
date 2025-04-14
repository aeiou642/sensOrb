#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include "NdefMessage.h"
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
uint8_t ndefBuf[128];

void setup() {
    Serial.begin(9600);
    Serial.println("NFC Peer to Peer Example - Send Message");
}

void loop() {
    Serial.println("Send a message to Peer");
    
    NdefMessage message = NdefMessage();
    message.addTextRecord("This is sending a text message!");
    
    bool success = nfc.write(message);
    if(success){
        Serial.println("Success! Sent a message");
    } else {
        Serial.println("womp womp");
    }

    delay(3000);
}
