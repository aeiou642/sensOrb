
#include "Electroniccats_PN7150.h"
#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "MS5837.h"
#include "TCA9548A.h"
#include <Adafruit_BNO08x.h>
#include <stdlib.h>
#include <stdio.h>

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
char io[1000];
char acc_io[1000];

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

const char a = 7 + '0';

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
                            0x08,                                                                                        // Payload length
                            'T',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'n',                                                                                   // Language
                            'w', 'o', 'r', 'l', 'd',                                                                    // Message Payload
                            0x51,                                                                                       // MB/ME/CF/1/IL/TNF
                            0x01,                                                                                       // Type length (1 byte)
                            0x07,                                                                                       // Payload length
                            'T',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'n',
                            io[0], io[1], io[2], io[3]};                                                                // Message Payload

void setup() {
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin();

  I2CMux.begin(Wire);           
  I2CMux.closeAll();  

  Serial.begin(115200);
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

  int pressure_size = snprintf(io, sizeof(io), "Pressures: %d %d %d %d %d %d %d %d", pressures[0], pressures[1], pressures[2], pressures[3], pressures[4], pressures[5], pressures[6], pressures[7]);
  Serial.println(io);

  char accel_x[5];
  char accel_y[5];
  char accel_z[5];
  dtostrf(sensorValue.un.accelerometer.x, 4, 2, accel_x);
  dtostrf(sensorValue.un.accelerometer.y, 4, 2, accel_y);
  dtostrf(sensorValue.un.accelerometer.z, 4, 2, accel_z);

  char gyro_x[5];
  char gyro_y[5];
  char gyro_z[5];
  dtostrf(sensorValue.un.gyroscope.z, 4, 2, gyro_x);
  dtostrf(sensorValue.un.gyroscope.y, 4, 2, gyro_y);
  dtostrf(sensorValue.un.gyroscope.z, 4, 2, gyro_z);

  int accel_size = snprintf(acc_io, sizeof(acc_io), "Acceleration - x: %s, y: %s, z: %s; Gyro - x: %s, y: %s, z: %s",
  accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
  Serial.println(acc_io);

  if (nfc.isReaderDetected()) {
    Serial.println("\nReader detected!");
    Serial.println("Sending NDEF message...");

    const char newdefMessage[] =        {0x91,                                                                                       // MB/ME/CF/1/IL/TNF
                            0x01,                                                                                       // Type length (1 byte)
                            0x50,                                                                                       // Payload length
                            'T',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'n',                                                                                   // Language
                            acc_io[0], acc_io[1], acc_io[2], acc_io[3], acc_io[4], 
                            acc_io[5], acc_io[6], acc_io[7], acc_io[8], acc_io[9],
                            acc_io[10], acc_io[11], acc_io[12], acc_io[13], acc_io[14],
                            acc_io[15], acc_io[16], acc_io[17], acc_io[18], acc_io[19],
                            acc_io[20], acc_io[21], acc_io[22], acc_io[23], acc_io[24],
                            acc_io[25], acc_io[26], acc_io[27], acc_io[28], acc_io[29],
                            acc_io[30], acc_io[31], acc_io[32], acc_io[33], acc_io[34],
                            acc_io[35], acc_io[36], acc_io[37], acc_io[38], acc_io[39],
                            acc_io[40], acc_io[41], acc_io[42], acc_io[43], acc_io[44], 
                            acc_io[45], acc_io[46], acc_io[47], acc_io[48], acc_io[49],
                            acc_io[50], acc_io[51], acc_io[52], acc_io[53], acc_io[54],
                            acc_io[55], acc_io[56], acc_io[57], acc_io[58], acc_io[59],
                            acc_io[60], acc_io[61], acc_io[62], acc_io[63], acc_io[64],
                            acc_io[65], acc_io[66], acc_io[67], acc_io[68], acc_io[69],
                            acc_io[70], acc_io[71], acc_io[72], acc_io[73], acc_io[74], acc_io[75], acc_io[76],                                                             // Message Payload
                            0x51,                                                                                       // MB/ME/CF/1/IL/TNF
                            0x01,                                                                                       // Type length (1 byte)
                            0x36,                                                                                       // Payload length
                            'T',                                                                                        // Type -> 'T' for text, 'U' for URI
                            0x02,                                                                                       // Status
                            'e', 'n',
                            io[0], io[1], io[2], io[3], io[4], io[5], io[6], io[7], io[8], io[9], io[10],               // Message Payload
                            io[11], io[12], io[13], io[14], io[15],
                            io[16], io[17], io[18], io[19], io[20],
                            io[21], io[22], io[23], io[24], io[25],
                            io[26], io[27], io[28], io[29], io[30],
                            io[31], io[32], io[33], io[34], io[35],
                            io[36], io[37], io[38], io[39], io[40],
                            io[41], io[42], io[43], io[44], io[45],
                            io[46], io[47], io[48], io[49], io[50]};                                                                                       

    message.setContent((newdefMessage), sizeof(newdefMessage));
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
