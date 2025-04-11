#ifndef NFC_H_N
#define NFC_H_N

#include <Arduino.h>
#include <Wire.h>

// Maximum time to wait for an answer from the PN532
#define PN532_TIMEOUT 1000
// Total length of the buffer for sending/receiving responses
#define PN532_PACKBUFFSIZE 80 

#define PN532_PREAMBLE (0x00)
#define PN532_STARTCODE1 (0x00)
#define PN532_STARTCODE2 (0xFF)
#define PN532_POSTAMBLE (0x00)
#define PN532_HOSTTOPN532 (0xD4)
#define PN532_PN532TOHOST (0xD5)
// PN532 Commands
#define PN532_COMMAND_DIAGNOSE (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION (0x02)
#define PN532_COMMAND_GETGENERALSTATUS (0x04)
#define PN532_COMMAND_READREGISTER (0x06)
#define PN532_COMMAND_WRITEREGISTER (0x08)
#define PN532_COMMAND_READGPIO (0x0C)
#define PN532_COMMAND_WRITEGPIO (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE (0x10)
#define PN532_COMMAND_SETPARAMETERS (0x12)
#define PN532_COMMAND_SAMCONFIGURATION (0x14)
#define PN532_COMMAND_POWERDOWN (0x16)
#define PN532_COMMAND_RFCONFIGURATION (0x32)
#define PN532_COMMAND_RFREGULATIONTEST (0x58)
#define PN532_COMMAND_INJUMPFORDEP (0x56)
#define PN532_COMMAND_INJUMPFORPSL (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET (0x4A)
#define PN532_COMMAND_INATR (0x50)
#define PN532_COMMAND_INPSL (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU (0x42)
#define PN532_COMMAND_INDESELECT (0x44)
#define PN532_COMMAND_INRELEASE (0x52)
#define PN532_COMMAND_INSELECT (0x54)
#define PN532_COMMAND_INAUTOPOLL (0x60)
#define PN532_COMMAND_TGINITASTARGET (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES (0x92)
#define PN532_COMMAND_TGGETDATA (0x86)
#define PN532_COMMAND_TGSETDATA (0x8E)
#define PN532_COMMAND_TGSETMETADATA (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS (0x8A)
// wakey wakey
#define PN532_WAKEUP (0x55)
// I2C
#define PN532_I2C_ADDRESS (0x48 >> 1)
#define PN532_I2C_READY (0x01)
// GPIO
#define PN532_GPIO_P30 (0x01)
#define PN532_GPIO_P31 (0x02)
#define PN532_GPIO_P32 (0x04)
#define PN532_GPIO_P33 (0x08)
#define PN532_GPIO_P34 (0x10)
#define PN532_GPIO_P35 (0x20)
#define PN532_GPIO_VALIDATIONBIT (0x80)

class PN532
{
    public:
    uint8_t resetPin;
    PN532(uint8_t rstP); // mostly unutilized
    void begin();
    bool getVersion(uint8_t* ICTYPE, uint8_t* MSB, uint8_t* LSB, uint8_t* FLAGS);
    bool checkStatus(uint8_t status);
    bool isReady();
    bool waitReady();
    bool waitReady(uint16_t timeWait);

    uint8_t tgInitAsTarget(); 
    uint8_t tgInitAsInitiator();
    
    uint8_t P2PInitiatorTxRx();
    uint8_t P2PTargetTxRx();


    private:
    bool readData(uint8_t* buf, uint8_t len);
    bool readAck();
    uint8_t writeCommandCheckAck(uint8_t* buf, uint8_t len);
    void sendPacket(uint8_t* buf, uint8_t len);
    void writeCommand(uint8_t* buf, uint8_t len);
    bool readPacket(uint8_t* buf, uint8_t len);
};
#endif