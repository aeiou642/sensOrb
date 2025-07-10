#include "emulatetag.h"
#include "NdefMessage.h"
#include <SPI.h>
#include <PN532_SPI.h>  
#include <PN532.h>

#define RST_PIN PIN_PA5  
#define LED_PIN PIN_PB4

PN532_SPI pn532spi(SPI, PIN_PA4);  // SS = PA4
PN532 nfc(pn532spi);

uint8_t ndefBuf[120];
uint8_t uid[3] = {0x12, 0x34, 0x56};
uint8_t num[4]{0,1,0,1};

void setup() {
  Serial.begin(115200);
  // if you don't reset the PN532 here, then it can infinitely hang for some reason?
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(100);
  digitalWrite(RST_PIN, HIGH);
  delay(100);
  // set the LED pin to output then low
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pn532spi.begin(); 
}

void loop() {
  for(int i = 0; i < 4; i++){
    num[i]++;
  };

  uint8_t success;
  uint8_t uid[] = {0,0,0,0,0,0,0};
  uint8_t uid_len;
  bool auth = false;
  // Check the NDEF keys, these should be the defaults
  // If it's a MiFare classic, should be just FF (x6)
  uint8_t keya[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
  uint8_t keyb[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
  //TODO - implement a physical button check since platformio doesn't like serial input
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_len);
  if(success){
    if(uid_len != 4){
      Serial.println("Not a MIFARE Classic card");
      return;
    };
    // block 4 is the first block with actual data, for more info, see 
    // this article (it is in german, good luck) 
    // https://www-sol6-de.translate.goog/tech/Mifare_Classic_-_NDEF/?_x_tr_sl=de&_x_tr_tl=en&_x_tr_hl=en&_x_tr_pto=wapp
    success = nfc.mifareclassic_AuthenticateBlock(uid, uid_len, 4,0, keyb);
    success = nfc.mifareclassic_WriteNDEFURI(1,0,"TTTTTTTTTTTTTTTTTTTT");
  };
}