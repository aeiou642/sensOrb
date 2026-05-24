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
  }

  // NFC configuration
  Serial.println("Initializing PN7150/PN7160...");
 
  nfc.setReadMsgCallback(messageReceivedCallback);
  nfc.setSendMsgCallback(messageSentCallback);

  // Start in read/write dependent on if I/O is present(determines if this is a sensor node)
  if(valid_io){
    setupWriterMode();
  }else{
    setupReaderMode();
  }

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
        case nfc.protocol.T2T:
        case nfc.protocol.T3T:
        case nfc.protocol.ISODEP:
        case nfc.protocol.MIFARE:
        case nfc.protocol.ISO15693:

          nfc.readNdefMessage();
          break;

        default:
          break;
      }
      Serial.println("Remove card...");
      nfc.waitForTagRemoval();

      Serial.println("Card removed");

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

  if (nfc.connectNCI()) { // Wake up board
    Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }

  if (nfc.configureSettings()) {
    Serial.println("The Configure Settings failed!");
    while (1);
  }

  if(nfc.ConfigMode(mode)){ 
    Serial.println("The Configure Mode failed!!");
    while (1);
  }
  nfc.StartDiscovery(mode);

  message.begin();
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

  message.begin();
  nfc.setEmulationMode(); 

  digitalWrite(PB15, HIGH);
  digitalWrite(PA13, LOW);

  Serial.println("Writer/Emulation mode active");
}

// Store new instance of data from barometers and IMU
void updateLocalSensorData() {

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

  bno08x.getSensorEvent(&sensorValue);

  // Create LOCAL node data
  snprintf(
    localData,
    sizeof(localData),
    "NODE[%lu] P:%d,%d,%d,%d,%d AX:%.2f AY:%.2f AZ:%.2f GX:%.2f GY:%.2f GZ:%.2f",
    millis(),
    pressures[0],
    pressures[1],
    pressures[2],
    pressures[3],
    pressures[4],
    sensorValue.un.accelerometer.x,
    sensorValue.un.accelerometer.y,
    sensorValue.un.accelerometer.z,
    sensorValue.un.gyroscope.x,
    sensorValue.un.gyroscope.y,
    sensorValue.un.gyroscope.z
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

  char ndef[1024];

  int payloadLen = strlen(chainBuffer);

  int index = 0;

  // NDEF TEXT RECORD HEADER
  ndef[index++] = 0xD1; // MB/ME/SR/TNF - NDEF format header
  ndef[index++] = 0x01; // Type length
  ndef[index++] = payloadLen + 3; // Payload length

  ndef[index++] = 'T';

  // Language code
  ndef[index++] = 0x02; // Length of language code
  ndef[index++] = 'e';
  ndef[index++] = 'n';

  // Copy payload
  memcpy(&ndef[index], chainBuffer, payloadLen);
  index += payloadLen;
  
  message.setContent(ndef, index);

  Serial.println("\nBroadcasting chain:");
  Serial.println(chainBuffer);

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

  do {

    record.create(message.getRecord());

    if (record.getType() == record.type.WELL_KNOWN_SIMPLE_TEXT) {

      String incoming = record.getText();

      Serial.println("\nRECEIVED CHAIN:");
      Serial.println(incoming);

      // Overwrite the existing chain buffer with the new incoming one

      memset(chainBuffer, 0, sizeof(chainBuffer));

      incoming.toCharArray(
        chainBuffer,
        sizeof(chainBuffer)
      );

      // If this is a sensor node, append the local data to the chain

      if(valid_io){
        updateLocalSensorData();
        appendLocalDataToChain();
      }

      Serial.println("\nCHAIN AFTER APPEND:");
      Serial.println(chainBuffer);
    }

  } while (record.isNotEmpty());
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