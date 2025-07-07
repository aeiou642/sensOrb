/*#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>

#define RST_PIN PIN_PA5
#define SS_PIN  PIN_PA4  // Adjust this to your SPI CS pin

PN532_SPI pn532spi(SPI, SS_PIN);
PN532 nfc(pn532spi);

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial
  Serial.println("PN532 NFC Reader");

  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(100);
  digitalWrite(RST_PIN, HIGH);
  delay(100);

  pn532spi.begin();
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN532");
    while (1);
  }

  Serial.print("Found PN532 firmware version: ");
  Serial.print((versiondata >> 24) & 0xFF, HEX);
  Serial.print('.');
  Serial.println((versiondata >> 16) & 0xFF, HEX);

  nfc.SAMConfig(); // configure board to read RFID tags
  Serial.println("Waiting for an ISO14443A card...");
}

void loop() {
  uint8_t success;
  uint8_t uid[7];
  uint8_t uidLength;

  // Wait for an ISO14443A type card (Mifare, your emulated tag, etc.)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);

  if (success) {
    Serial.print("Found a card UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Optionally, you can try to read data blocks here using inDataExchange() or mifareclassic commands
  } else {
    Serial.println("No card detected");
  }
  delay(1000);
*/
#include "emulatetag.h"
#include "NdefMessage.h"
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>

#define RST_PIN PIN_PA5  
#define LED_PIN PIN_PB4

PN532_SPI pn532spi(SPI, PIN_PA4);  // SPI CS = PA4
EmulateTag nfc(pn532spi);

uint8_t ndefBuf[120];
uint8_t uid[3] = {0x12, 0x34, 0x56};  // 3-byte UID (must be 3 bytes for EmulateTag)

NdefMessage message;
int messageSize;

void setup() {
  Serial.begin(115200);
  Serial.println("------- PN532 Tag Emulation --------");

  // Reset PN532
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delay(100);
  digitalWrite(RST_PIN, HIGH);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pn532spi.begin();
  
  // Create NDEF message with a URI record
  message = NdefMessage();
  message.addUriRecord("https://web.engr.oregonstate.edu/~johnstom/");

  messageSize = message.getEncodedSize();
  if (messageSize > sizeof(ndefBuf)) {
    Serial.println("ndefBuf is too small");
    while (1);
  }

  message.encode(ndefBuf);
  
  // Set NDEF message and UID for the emulated tag
  nfc.setNdefFile(ndefBuf, messageSize);
  nfc.setUid(uid);  // UID length must be 3 bytes

  if (!nfc.init()) {
    Serial.println("Failed to initialize emulated tag!");
    while(1);
  }
}

void loop() {
  // Reload NDEF file in case it was written to previously
  nfc.setNdefFile(ndefBuf, messageSize);

  // Emulate tag with 1000ms timeout
  if (!nfc.emulate(1000)) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Emulation timed out.");
  } else {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Tag read or written.");
    delay(1000);
  }

  // Check if a write to the tag happened
  if (nfc.writeOccured()) {
    Serial.println("\nWrite occurred!");

    uint8_t* tag_buf;
    uint16_t length;
    nfc.getContent(&tag_buf, &length);
    
    // Decode and print the updated NDEF message
    NdefMessage msg = NdefMessage(tag_buf, length);
    msg.print();
  }
  
  delay(1000);
}