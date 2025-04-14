#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include "NdefMessage.h"
#include <NfcAdapter.h>

#define NDEF_DEBUG

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
uint8_t ndefBuf[128];

void setup() {
    Serial.begin(9600);
    Serial.println("Receive Message");
    nfc.begin();
}

void loop() {
    Serial.println("Waiting for message from Peer");
    if (nfc.tagPresent()){
        NfcTag tag = nfc.read();
        tag.print();
    }
    delay(1000);
}

