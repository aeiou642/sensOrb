/**
 * Example to create a custom NDEF message and send it using the PN7150.
 *
 * Note: If you know how to create custom NDEF messages, you can use this example, otherwise, use the NDEFSendMessage example.
 *
 * Authors:
 *        Salvador Mendoza - @Netxing - salmg.net
 *        Francisco Torres - Electronic Cats - electroniccats.com
 *
 * December 2023
 *
 * This code is beerware; if you see me (or any other collaborator
 * member) at the local, and you've found our code helpful,
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#include "Electroniccats_PN7150.h"
#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "MS5837.h"
#include "TCA9548A.h"
#include <Adafruit_BNO08x.h>

MS5837 sensor0;
MS5837 sensor1;
MS5837 sensor2;
MS5837 sensor3;
MS5837 sensor4;
MS5837 sensor5;
MS5837 sensor6;
MS5837 sensor7;

MS5837 sensor_arr[8] = {sensor0, sensor1, sensor2, sensor3, sensor4, sensor5, sensor6, sensor7};
int pressures[8] = {};

TCA9548A I2CMux;

// For SPI mode, we need a CS pin
//#define BNO08X_CS 10
//#define BNO08X_INT 9

// For SPI mode, we also need a RESET
//#define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;
void setReports(void);

#define PN7150_IRQ (PA3)
#define PN7150_VEN (PA4)
#define PN7150_ADDR (0x28)

// Function prototypes
void messageSentCallback();

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR, PN7160); // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28,specify PN7150 or PN7160 in constructor

NdefMessage message;

// Three records, "Hello", "world" and Uri "https://www.electroniccats.com"
const char ndefMessage[] =        {0x91,                                                                                       // MB/ME/CF/1/IL/TNF
                            0x01,                                                                                       // Type length (1 byte)
                            0x08,                                                                                       // Payload length
                            'T',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'n',                                                                                   // Language
                            'H', 'e', 'l', 'l', 'o',                                                                    // Message Payload
                            0x11,                                                                                       // MB/ME/CF/1/IL/TNF
                            0x01,                                                                                       // Type length (1 byte)
                            0x08,                                                                                       // Payload length
                            'T',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'n',                                                                                   // Language
                            'w', 'o', 'r', 'l', 'd',                                                                    // Message Payload
                            0x51,                                                                                       // MB/ME/CF/1/IL/TNF
                            0x01,                                                                                       // Type length (1 byte)
                            0x13,                                                                                       // Payload length
                            'U',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'l', 'e', 'c', 't', 'r', 'o', 'n', 'i', 'c', 'c', 'a', 't', 's', '.', 'c', 'o', 'm'};  // Message Payload

void setup() {
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin();

  I2CMux.begin(Wire);           
  I2CMux.closeAll();  

  Serial.begin(9600);
  while (!Serial);

  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }

  setReports();

  Serial.println("Reading events");
  delay(100);

  Serial.println("Send NDEF Message with PN7150/60");

  message.setContent(ndefMessage, sizeof(ndefMessage));
  nfc.setSendMsgCallback(messageSentCallback);

  Serial.println("Initializing...");

  if (nfc.begin()) {
    Serial.println("Error initializing PN7150");
    while (true)
      ;
  }

  // Needed to detect readers
  nfc.setEmulationMode();
  Serial.print("Waiting for an NDEF device");

  for(int i = 0; i < 8; i++){
    I2CMux.openChannel(i);

    // Initialize pressure sensor
    // Returns true if initialization was successful
    // We can't continue with the rest of the program unless we can initialize the sensor
    while (!sensor_arr[i].init()) {
      Serial.println("Init failed!");
      Serial.println("Are SDA/SCL connected correctly?");
      Serial.println("\n\n\n");
      delay(1000);
    }

    // .init sets the sensor model for us but we can override it if required.
    // Uncomment the next line to force the sensor model to the MS5837_30BA.
    sensor_arr[i].setModel(MS5837::MS5837_30BA);
    sensor_arr[i].setFluidDensity(997); // kg/m^3 (freshwater, 1029 for seawater)
    I2CMux.closeChannel(i);
  }


}

void loop() {
  Serial.print(".");

  for(int i = 0; i < 8; i++){
    I2CMux.openChannel(i);
    sensor_arr[i].read();

    pressures[i] = sensor_arr[i].pressure();
    I2CMux.closeChannel(i);
  }

  if (bno08x.wasReset()) {
    Serial.println("\nBN0 Sensor was reset.");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  if (nfc.isReaderDetected()) {
    Serial.println("\nReader detected!");
    Serial.println("Sending NDEF message...");
    
    char io_message_data[1000];

    int size = snprintf(io_message_data, sizeof(io_message_data), "Pressures: %d %d %d %d %d %d %d %d", pressures[0], pressures[1], pressures[2], pressures[3], pressures[4], pressures[5], pressures[6], pressures[7]);
    Serial.println(io_message_data);

    const char* new_ndefMessage = io_message_data;
    message.setContent((new_ndefMessage), sizeof(new_ndefMessage));
    nfc.sendMessage();
    Serial.print("\nWaiting for an NDEF device");
  }
}

void messageSentCallback() {
  Serial.println("NDEF message sent!");
  // Do something...
}


void setReports(void) {
  Serial.println("\nSetting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
    Serial.println("Could not enable gyroscope");
  }
}
