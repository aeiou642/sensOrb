#include "PN532_NEW.h"

uint8_t packetBuffer[PN532_PACKBUFFSIZE];

PN532::PN532(uint8_t rstP)
{
    resetPin = rstP;
}

/*!
    @brief Begins communication. Resets, then calls TWI/I2C Wire constructor to start 
    @param NONE
    @return NONE
*/
void PN532::begin(void)
{
    digitalWrite(resetPin, HIGH);
    delay(10);
    digitalWrite(resetPin, LOW);
    delay(400);
    digitalWrite(resetPin, HIGH);
    delay(10);
    Wire.begin(); // i'm using I2C headers out of preference
    Wire.setClock(400000); // run I2C at fast mode
}
/*!
    @brief Get the version of the PN532 chip and print it to serial
    @return NONE
*/
bool PN532::getVersion(uint8_t* ICTYPE, uint8_t* MSB, uint8_t* LSB, uint8_t* FLAGS){
    packetBuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    if(!writeCommandCheckAck(packetBuffer, PN532_PACKBUFFSIZE)){
        return 0;
    }
    byte len = readData(packetBuffer, 13);
    if(len != 6 || packetBuffer[1] != PN532_COMMAND_GETFIRMWAREVERSION + 1){
        return 0;
    }
}

/*!
    @brief Check to see if the PN532 is ready. This is done by 
            using the
    @return Returns true if ready.
*/
bool isReady(){
    // Once read, release with a stop condition
    // Additionally, note that before reading data, check for isReady (6.2.4)
    Wire.requestFrom((uint8_t)PN532_I2C_ADDRESS,(uint8_t)1);
    uint8_t ready = Wire.read();
    return ready == PN532_I2C_READY;
}

/*!
    @brief Waits until the PN532 is ready. As a note, a timeout here is needed,
            otherwise the program will hang here, as we've found out.
    @param PN532_TIMEOUT Macro. Default 1000 (miliseconds)
    @return Returns true once the PN532 is ready. If the time exceeds PN532_TIMEOUT,
            then returns false. 
*/
bool waitReady(){
    uint16_t timer = 0;
    while(!isReady()){
        if(timer >= PN532_TIMEOUT){
            return false;
        }
        delay(10);
        timer+=10;
    }
    return true;
}
/*!
    @brief Waits until the PN532 is ready. As a note, a timeout here is needed,
            otherwise the program will hang here, as we've found out.
    @param timeWait How long to wait in miliseconds
    @return Returns true once the PN532 is ready. If the time exceeds timeWait,
            then returns false. 
*/
bool waitReady(uint16_t timeWait){
    uint16_t timer = 0;
    while(!isReady()){
        if(timer >= timeWait){
            return false;
        }
        delay(10);
        timer+=10;
    }
    return true;
}

bool PN532::readData(uint8_t* buf, uint8_t len){
    //TODO: add proper return values for more verbose debugging
    delay(2); // let the PN532 wake up
    uint8_t RXBUF[PN532_PACKBUFFSIZE];
    const uint8_t MIN_PACK_LEN = 2;

    Wire.requestFrom(uint8_t(PN532_I2C_ADDRESS),len);
    if(!readPacket(RXBUF,len)){
        return 0;        
    }
    for(uint8_t i = 0; i < len; i++){
        delay(1);
        buf[i] = readPacket(buf, len);
    }
    const char *error = NULL;
    int b1 = -1; // Lower bound for data
    int pos = 0; // Position pointer for the array
    int b2 = -1; // Upper bound for data
    int dL = 0; // Length of data
    int startCode = -1;
    do{
    for(int i = 0; i <= len - MIN_PACK_LEN; i++){
        if(RXBUF[i] == PN532_STARTCODE1 && 
            RXBUF[i+1] == PN532_STARTCODE2){
            startCode = i;
            break;
        }
    }
    if (startCode < 0 ){
        break; // No start code read
    }
    pos = startCode + 2;
    dL = RXBUF[pos++];
    int lC = RXBUF[pos++];
    if((dL + lC) != 0x100){
        break; // invalid checksum
    }
    if(len < (startCode + MIN_PACK_LEN + dL)){
        break; // Packet is longer than expected length
    };
    b1 = pos;
    for(int i = 0; i < dL; i++){
        // Copy data bytes
        buf[i] = RXBUF[pos++];
    }
    b2 = pos;

    // Data sent to host starts with 0xD5 (PN532TOHOST)
    


    } while (false);
}

/*!
    @brief Read a packet of information. Note that this explicity does NOT
            check for valid data. 
    @param buf Pointer to the buffer where data will be written
    @param len Length of either bytes to be read, or the size of the buffer.
    @return Returns true on success. False if the PN532 is not ready.
*/

bool PN532::readPacket(uint8_t* buf, uint8_t len){
    if(!waitReady()){
        return false;
    }
    // block and wait before reading
    delay(2);
    Wire.requestFrom(uint8_t(PN532_I2C_ADDRESS),uint8_t(len+1));
    uint8_t ready = Wire.read();
    for(int i = 0; i < len; i++){
        delay(1);
        buf[i] = Wire.read();
    } 
    return true;
}
/*!
    @brief Read the acknowledge (ACK) packet. Adafruit utilize this...
            for some reason.
    @return Returns true on ACK frame received.
*/
bool PN532::readAck(){
    const uint8_t ACK[]{
        0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00
    };
    uint8_t ackBuf[sizeof(ACK)];
    if(!readPacket(ackBuf, sizeof(ACK))){
        return false;
    }
    if(memcmp(ackBuf, ACK, sizeof(ACK))){
        return false;
    }

}


/*!
    @brief Writes a command with proper proceeding and ending bits (s2.2.1 AN). 
            Utilizes sendPacket to send the buffer over once constructing the
            frame.
    @param cmd uint8_t, cmd[0] should be the command, following by data
    @param cmdlen size_t, array length
    @return None

*/
void PN532::writeCommand(uint8_t* buf, uint8_t len){
    uint8_t writeBuffer[PN532_PACKBUFFSIZE + 10];
    uint8_t checkSum = 0;

    int P = 0;
    writeBuffer[P++] = PN532_PREAMBLE;
    writeBuffer[P++] = PN532_STARTCODE1;
    writeBuffer[P++] = PN532_STARTCODE2;
    writeBuffer[P++] = len + 1;
    writeBuffer[P++] = 0xFF - len;
    writeBuffer[P++] = PN532_HOSTTOPN532;
    
    for(uint8_t i = 0; i < len; i++){
        writeBuffer[P++] = buf[i];
    };
    for(uint8_t i = 0; i < P; i ++){
        checkSum += writeBuffer[i];
    }
    writeBuffer[P++] = ~checkSum;
    writeBuffer[P++] = PN532_POSTAMBLE;
    sendPacket(writeBuffer, P);
}

/*!
    @brief Using TWI/I2C, write a buffer to the line, given a buffer and length.
    @param buf uint8_t*
    @param len uint8_t (change to size_t later), length of buf
*/
void PN532::sendPacket(uint8_t* buf, uint8_t len){
    delay(2);
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    for(uint8_t i = 0; i < len; i++){
        Wire.write(buf[i]);
    }
    Wire.endTransmission();
}

/*!
    @brief writeCommand, but check ACK
            as a note, Adafruit code does not do this  
    @param buf - buffer, first bit command
    @param len - length of buffer
    @return readAck() status
*/
uint8_t PN532::writeCommandCheckAck(uint8_t* buf, uint8_t len){
    writeCommand(buf, len);
    waitReady();
    return readAck();
};

/*!
    @brief Initialize the PN532 as a target, for peer-to-peer communication.
            First, builds a buffer with the command, and all the parameters
            for peer-to-peer (see s7.3.14, UM). Then, calls writeCommand to
            write it over. Note for our usecase, we only care about P2P(DEP)
            communication.
    @param NONE
    @return 0 - Failure,      
    @return 1 - Success
*/
uint8_t PN532::tgInitAsTarget(){
    static uint8_t send_flag = 0; // don't resend this
    NFC_BUFFER[0] = PN532_COMMAND_TGINITASTARGET;
    NFC_BUFFER[1] = 0x00; // i literally cannot get my hands on ISO/IEC 14443-3:2018; it's $230 USD
    NFC_BUFFER[2] = 0x04; // so the rest of this is based off of the ELECHOUSE and Adafruit code
    NFC_BUFFER[3] = 0x00; // SENS_RES
    NFC_BUFFER[4] = 0x12; // NFCID1
    NFC_BUFFER[5] = 0x34; // NFCID1
    NFC_BUFFER[6] = 0x56; // NFCID1
    NFC_BUFFER[7] = 0x40; // SEL_RES, DEP only mode
    NFC_BUFFER[8] = 0x01; // POL_RES
    NFC_BUFFER[9] = 0xFE; // POL_RES
    NFC_BUFFER[10] = 0xA2; // POL_RES
    NFC_BUFFER[11] = 0xA3; // POL_RES
    NFC_BUFFER[12] = 0xA4; // POL_RES
    NFC_BUFFER[13] = 0xA5; // POL_RES
    NFC_BUFFER[14] = 0xA6; // POL_RES
    NFC_BUFFER[15] = 0xA7; // POL_RES
    NFC_BUFFER[16] = 0xC0; // POL_RES
    NFC_BUFFER[17] = 0xC1; // POL_RES
    NFC_BUFFER[18] = 0xC2; // POL_RES
    NFC_BUFFER[19] = 0xC3; // POL_RES
    NFC_BUFFER[20] = 0xC4; // POL_RES
    NFC_BUFFER[21] = 0xC5; // POL_RES
    NFC_BUFFER[22] = 0xC6; // POL_RES
    NFC_BUFFER[23] = 0xC7; // POL_RES
    NFC_BUFFER[24] = 0xFF; // POL_RES
    NFC_BUFFER[25] = 0xFF; // POL_RES
    NFC_BUFFER[26] = 0xAA; // NFCID3t
    NFC_BUFFER[27] = 0x99; // NFCID3t
    NFC_BUFFER[28] = 0x88; // NFCID3t
    NFC_BUFFER[29] = 0x77; // NFCID3t
    NFC_BUFFER[30] = 0x66; // NFCID3t
    NFC_BUFFER[31] = 0x55; // NFCID3t
    NFC_BUFFER[32] = 0x44; // NFCID3t
    NFC_BUFFER[33] = 0x33; // NFCID3t
    NFC_BUFFER[34] = 0x22; // NFCID3t
    NFC_BUFFER[35] = 0x11; // NFCID3t
    NFC_BUFFER[36] = 0x00; // General uint8_t length
    NFC_BUFFER[37] = 0x00; // Historical uint8_t length
    if(!send_flag){
        send_flag = 1;
        if(!writeCommandCheckAck(NFC_BUFFER,38)){
            return 0;
        }
        return 1;
    }
}


bool PN532::checkStatus(uint8_t status)
{
    // Bits 0...5 contain the error code.
    status &= 0x3F;
    if (status == 0)
        return true;
    switch (status)
    {
    case 0x01:
        Serial.println("Timeout\r\n");
        return false;
    case 0x02:
        Serial.println("CRC error\r\n");
        return false;
    case 0x03:
        Serial.println("Parity error\r\n");
        return false;
    case 0x04:
        Serial.println("Wrong bit count during anti-collision\r\n");
        return false;
    case 0x05:
        Serial.println("Framing error\r\n");
        return false;
    case 0x06:
        Serial.println("Abnormal bit collision\r\n");
        return false;
    case 0x07:
        Serial.println("Insufficient communication buffer\r\n");
        return false;
    case 0x09:
        Serial.println("RF buffer overflow\r\n");
        return false;
    case 0x0A:
        Serial.println("RF field has not been switched on\r\n");
        return false;
    case 0x0B:
        Serial.println("RF protocol error\r\n");
        return false;
    case 0x0D:
        Serial.println("Overheating\r\n");
        return false;
    case 0x0E:
        Serial.println("Internal buffer overflow\r\n");
        return false;
    case 0x10:
        Serial.println("Invalid parameter\r\n");
        return false;
    case 0x12:
        Serial.println("Command not supported\r\n");
        return false;
    case 0x13:
        Serial.println("Wrong data format\r\n");
        return false;
    case 0x14:
        Serial.println("Authentication error\r\n");
        return false;
    case 0x23:
        Serial.println("Wrong UID check uint8_t\r\n");
        return false;
    case 0x25:
        Serial.println("Invalid device state\r\n");
        return false;
    case 0x26:
        Serial.println("Operation not allowed\r\n");
        return false;
    case 0x27:
        Serial.println("Command not acceptable\r\n");
        return false;
    case 0x29:
        Serial.println("Target has been released\r\n");
        return false;
    case 0x2A:
        Serial.println("Card has been exchanged\r\n");
        return false;
    case 0x2B:
        Serial.println("Card has disappeared\r\n");
        return false;
    case 0x2C:
        Serial.println("NFCID3 initiator/target mismatch\r\n");
        return false;
    case 0x2D:
        Serial.println("Over-current\r\n");
        return false;
    case 0x2E:
        Serial.println("NAD msssing\r\n");
        return false;
    default:
        Serial.println("Undocumented error\r\n");
        return false;
    }
}
