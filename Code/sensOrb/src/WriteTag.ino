//Modfied example code

  #define NFC_INTERFACE_SPI
  #include <SPI.h>
  #include <PN532_SPI.h>
  #include <PN532_SPI.cpp>
  #include "PN532.h"

  PN532_SPI pn532spi(SPI, PIN_PA4);
  PN532 nfc(pn532spi);

uint8_t ndefprefix = NDEF_URIPREFIX_NONE;
void setup(void) {
  Serial.begin(115200);
  Serial.println("Looking for PN532...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();
}

void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  bool authenticated = false;               // Flag to indicate if the sector is authenticated

  // Use the default key
  uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success)
  {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      Serial.print(' ');
    }
    Serial.println("");

    // Make sure this is a Mifare Classic card
    if (uidLength != 4)
    {
      Serial.println("Not a MIFARE Classic card");
      return;
    }

    // We probably have a Mifare Classic card ...
    Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
    // Try to format the card for NDEF data
    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    success = nfc.mifareclassic_FormatNDEF();
    Serial.println("Card has been formatted for NDEF data using MAD1");
    // Try to authenticate block 4 (first block of sector 1) using our key
    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 4, 0, keya);
    Serial.println("Writing URI to sector 1 as an NDEF Message");
    success = nfc.mifareclassic_WriteNDEFURI(1, ndefprefix, "TTTTTTTTTTT");
    if (success)
    {
      Serial.println("NDEF URI Record written to sector 1");
    }
    else
    {
      Serial.println("NDEF Record creation failed!");
    }
  }
}