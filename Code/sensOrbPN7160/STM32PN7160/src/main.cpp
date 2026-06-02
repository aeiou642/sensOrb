/*
  NFC Sensor Node Signal Chaining Code [SENSORB]
  Author: Jack Cunningham
  Date: 5/23/26

  This code operates several modules in tandem to produce
  a NFC based relay of information from a head sensor node to
  following reader nodes and back.

  This code handles:
  - NFC reader/writer and emulation modes of an integrated PN7160 
  - NDEF Data block decoding/encoding operations
  - TCA9548A multiplexer channel selection for:
    - Reading pressure data of five MS5837 Barometers
  - Reading data of Adafruit BNO085 IMU-Sensor fusion
  - Relaying above sensor data across NFC nodes

*/


// Libraries necessary for operation
#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <stdlib.h>
#include <stdio.h>
#include "Electroniccats_PN7150.h"
#include "MS5837.h"
#include "TCA9548A.h"
#include <Adafruit_BNO08x.h>

// NFC pin definitions and objects
#define PN7160_IRQ  (PA4)
#define PN7160_VEN  (PA5)
#define PN7160_ADDR (0x28)
Electroniccats_PN7150 nfc(PN7160_IRQ, PN7160_VEN, PN7160_ADDR, PN7160);
NdefMessage message;

// Barometer array
MS5837 sensor0;
MS5837 sensor1;
MS5837 sensor2;
MS5837 sensor3;
MS5837 sensor4;

MS5837 sensor_arr[5] = {
  sensor0, sensor1, sensor2, sensor3, sensor4
};

int pressures[5] = {};  // Pressure data from barometers

char chainBuffer[1024];     // Chained message to send out
char ndefBuffer[1024];
char localData[256];        // This node's local sensor data

TCA9548A I2CMux;

#define BNO08X_RESET -1
Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

int valid_io = 1; // 1 = Sensor node, 0 = Repeater node
uint8_t mode = 2; // 1 = R/W Mode(Reading Operation), 2 = Emulation Mode(Writing Operation)

// Function prototypes
void setupReaderMode();
void setupWriterMode();

void messageReceivedCallback();
void messageSentCallback();

void displayDeviceInfo();
void displayRecordInfo(NdefRecord record);
String getHexRepresentation(const byte *data, const uint32_t dataSize);

void setReports(void);
void updateLocalSensorData();
void appendLocalDataToChain();
void sendChainNDEF();

// Node setup for operation
void setup() {
  delay(1000); // Power up delay

  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin();
  Serial.begin(115200);

  pinMode(PA13, OUTPUT); // LED configuration
  pinMode(PB15, OUTPUT);

  I2CMux.begin(Wire); // TCA9548A 8:1 setup
  I2CMux.closeAll();

  // IMU Initialization, and if not detected, clear valid_io
  Serial.println("Initializing BNO08x...");
  if (!bno08x.begin_I2C()) {
    Serial.println("Failed to find BNO08x");
    valid_io = 0;
    mode = 1;
  }

  if(valid_io){
    setReports();  // IMU configuration
    // Mux and pressure sensor configuration
    for (int i = 0; i < 5; i++) {

      I2CMux.openChannel(i);

      if (!sensor_arr[i].init()) {
        Serial.print("Pressure sensor init failed on channel ");
        Serial.println(i);
      }
    
      sensor_arr[i].setModel(MS5837::MS5837_30BA);
      sensor_arr[i].setFluidDensity(997);
    
      I2CMux.closeChannel(i);
    }
    Serial.print("Setup of IO complete.");
  }

  // NFC configuration
  Serial.println("Initializing PN7150/PN7160...");
 
  //nfc.setReadMsgCallback(messageReceivedCallback);
  //nfc.setSendMsgCallback(messageSentCallback);

  // Start in read/write dependent on if I/O is present(determines if this is a sensor node)
  if(valid_io){
    setupWriterMode();
  }else{
    setupReaderMode();
  }
  message.begin();

  Serial.println("System Ready");
}

// Looping operation for reading and writing to nodes
void loop() {

  // Writing operation
  if (mode == 2) {
    Serial.print(".");
    if (nfc.isReaderDetected()) {

      Serial.println("\nReader detected!");
      Serial.println("Sending NDEF...");

      if(valid_io){
        updateLocalSensorData();
        delay(1);
        Serial.println("Updated local data.");
        appendLocalDataToChain();
      }
      sendChainNDEF();

      digitalWrite(PA13, HIGH);
      delay(100);
      digitalWrite(PA13, LOW);
      delay(100);
      digitalWrite(PA13, HIGH);
      delay(100);
      digitalWrite(PA13, LOW);
      delay(100);
      digitalWrite(PA13, HIGH);
      delay(100);
      digitalWrite(PA13, LOW); 

      mode = 1;
      setupReaderMode();
    }
  }

  // Reading operation
  else if (mode == 1) {
    Serial.print(",");
    if (nfc.isTagDetected()) {

      Serial.println("\nNFC Tag detected");

      displayDeviceInfo();

      switch (nfc.remoteDevice.getProtocol()) {

        case nfc.protocol.T1T:
          nfc.readNdefMessage();
          break;
        case nfc.protocol.T2T:
          nfc.readNdefMessage();
          break;
        case nfc.protocol.T3T:
          nfc.readNdefMessage();
          break;
        case nfc.protocol.ISODEP:
          nfc.readNdefMessage();
          break;
        case nfc.protocol.MIFARE:
          nfc.readNdefMessage();
          break;
        case nfc.protocol.ISO15693:
          nfc.readNdefMessage();
          break;
        default:
          break;
      }
      Serial.println("Remove card...");
      nfc.waitForTagRemoval();

      Serial.println("Card removed");

      messageReceivedCallback();

      digitalWrite(PB15, HIGH);
      delay(100);
      digitalWrite(PB15, LOW);
      delay(100);
      digitalWrite(PB15, HIGH);
      delay(100);
      digitalWrite(PB15, LOW);
      delay(100);
      digitalWrite(PB15, HIGH);
      delay(100);
      digitalWrite(PB15, LOW);

      mode = 2;
      setupWriterMode();
    }
  }
  delay(1);
}

// Reader mode configuration helper function
void setupReaderMode() {

  mode = 1;

  nfc.connectNCI();
  nfc.configureSettings();
  
  nfc.ConfigMode(mode);
  nfc.StartDiscovery(mode);

  nfc.setReaderWriterMode();

  digitalWrite(PB15, LOW);
  digitalWrite(PA13, HIGH);

  Serial.println("Reader mode active");
}

// Writer mode configuration helper function
void setupWriterMode() {
  
  mode = 2;

  nfc.connectNCI();
  nfc.configureSettings();

  nfc.ConfigMode(mode);
  nfc.StartDiscovery(mode);

  nfc.setEmulationMode(); 

  digitalWrite(PB15, HIGH);
  digitalWrite(PA13, LOW);

  Serial.println("Writer/Emulation mode active");
}

// Store new instance of data from barometers and IMU
void updateLocalSensorData() {
  Serial.print("Updating node sensor data.... ");
  // Read pressure sensors
  for (int i = 0; i < 5; i++) {

    I2CMux.openChannel(i);

    sensor_arr[i].read();

    pressures[i] = sensor_arr[i].pressure();

    I2CMux.closeChannel(i);
  }

  // Read IMU
  if (bno08x.wasReset()) {
    setReports();
  }
  while (!bno08x.getSensorEvent(&sensorValue)) {
  }
  
  char accel_x[8];
  char accel_y[8];
  char accel_z[8];
  dtostrf(sensorValue.un.accelerometer.x, 4, 2, accel_x);
  dtostrf(sensorValue.un.accelerometer.y, 4, 2, accel_y);
  dtostrf(sensorValue.un.accelerometer.z, 4, 2, accel_z);

  char gyro_x[8];
  char gyro_y[8];
  char gyro_z[8];
  dtostrf(sensorValue.un.gyroscope.x, 4, 2, gyro_x);
  dtostrf(sensorValue.un.gyroscope.y, 4, 2, gyro_y);
  dtostrf(sensorValue.un.gyroscope.z, 4, 2, gyro_z);

  // Create LOCAL node data
  int len = snprintf(
    localData,
    sizeof(localData),
    "NODE[%lu]-P:%d,%d,%d,%d,%d;AX:%s,AY:%s,AZ:%s,GX:%s,GY:%s,GZ:%s",
    millis(),
    pressures[0],
    pressures[1],
    pressures[2],
    pressures[3],
    pressures[4],
    accel_x,
    accel_y,
    accel_z,
    gyro_x,
    gyro_y,
    gyro_z
  );

  Serial.println(localData);
}

// Convert sensor data and append to the chain
void appendLocalDataToChain() {

  // First node case
  if (strlen(chainBuffer) == 0) {

    snprintf(
      chainBuffer,
      sizeof(chainBuffer),
      "%s",
      localData
    );
  }

  // Append new hop
  else {

    strncat(
      chainBuffer,
      " -> ",
      sizeof(chainBuffer) - strlen(chainBuffer) - 1
    );

    strncat(
      chainBuffer,
      localData,
      sizeof(chainBuffer) - strlen(chainBuffer) - 1
    );
  }

  Serial.println("\nUPDATED CHAIN:");
  Serial.println(chainBuffer);
}

// Convert data chain to NFC Data Exchange Format(NDEF)
void sendChainNDEF() {

  //char ndef[1024];

  int payloadLen = strlen(chainBuffer);
  Serial.print("Payload length = ");
  Serial.println(payloadLen);

  int index = 0;

  // NDEF TEXT RECORD HEADER
  ndefBuffer[index++] = 0xD1; // MB/ME/SR/TNF - NDEF format header
  ndefBuffer[index++] = 0x01; // Type length
  ndefBuffer[index++] = payloadLen + 3; // Payload length

  ndefBuffer[index++] = 'T';

  // Language code
  ndefBuffer[index++] = 0x02; // Length of language code
  ndefBuffer[index++] = 'e';
  ndefBuffer[index++] = 'n';

  // Copy payload
  memcpy(&ndefBuffer[index], chainBuffer, payloadLen);
  index += payloadLen;
  
  message.setContent(ndefBuffer, index);

  Serial.println(message.getContentLength());

  uint8_t* p = message.getContent();

  for(int i=0;i<message.getContentLength();i++)
  {
    Serial.printf("%02X ", p[i]);
  }
  Serial.println();

  nfc.sendMessage();
}

// Callback on send, no necessary operation
void messageSentCallback() {
  Serial.println("NDEF Message Sent!");
}

// Callback on receive, append incoming data to chain
void messageReceivedCallback() {

  NdefRecord record;

  Serial.println("\nIncoming NFC payload");

  if (message.isEmpty()) {
    Serial.println("Empty message");
    return;
  }

  uint8_t* buf = message.getContent();
  size_t len = message.getContentLength();

  for(size_t i=0;i<len;i++) {
    Serial.printf("%02X ", buf[i]);
  }
  Serial.println();

  record.create(message.getRecord());

  if(record.getType() == record.type.WELL_KNOWN_SIMPLE_TEXT)
  {
    String incoming = record.getText();

    Serial.println(incoming);

    memset(chainBuffer, 0, sizeof(chainBuffer));
    incoming.toCharArray(chainBuffer, sizeof(chainBuffer));
  }
}

// NDEF helper function
String getHexRepresentation(const byte *data, const uint32_t dataSize) {

  String hexString;

  if (dataSize == 0) {
    return "null";
  }

  for (uint32_t i = 0; i < dataSize; i++) {

    if (data[i] <= 0xF)
      hexString += "0";

    String hexValue = String(data[i] & 0xFF, HEX);

    hexValue.toUpperCase();

    hexString += hexValue;

    if ((dataSize > 1) && (i != dataSize - 1)) {
      hexString += ":";
    }
  }

  return hexString;
}

// NDEF helper function
void displayDeviceInfo() {

  Serial.println();

  Serial.print("Protocol: ");

  switch (nfc.remoteDevice.getProtocol()) {

    case nfc.protocol.T1T:
      Serial.println("T1T");
      break;

    case nfc.protocol.T2T:
      Serial.println("T2T");
      break;

    case nfc.protocol.T3T:
      Serial.println("T3T");
      break;

    case nfc.protocol.ISODEP:
      Serial.println("ISO-DEP");
      break;

    case nfc.protocol.MIFARE:
      Serial.println("MIFARE");
      break;

    case nfc.protocol.ISO15693:
      Serial.println("ISO15693");
      break;

    default:
      Serial.println("UNKNOWN");
      break;
  }
}

// Ndef helper function
void displayRecordInfo(NdefRecord record) {

  if (record.isEmpty()) {
    return;
  }

  Serial.println("--- NDEF Record ---");

  switch (record.getType()) {

    case record.type.WELL_KNOWN_SIMPLE_TEXT:

      Serial.println("TEXT RECORD:");
      Serial.println(record.getText());

      break;

    case record.type.WELL_KNOWN_SIMPLE_URI:

      Serial.println("URI RECORD:");
      Serial.println(record.getUri());

      break;

    default:

      Serial.println("Unsupported record");

      break;
  }

  Serial.println();
}

// IMU report configuration, enable specified reports
void setReports(void) {

  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }

  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
    Serial.println("Could not enable gyroscope");
  }
}