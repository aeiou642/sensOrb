// Author - Owen Pyle
// Date - 2/27/2026
// Description:
/*
    Developed as part of a replacement/learning process for the PN532 I2C drivers.
    
    V0.1.1 - Added basic comments and explanation regarding standard information frame
    V0.1.2 - Added basic member functions, no functionality yet
*/

#ifndef NFC_H
#endif NFC_H

#include <avr/pgmspace.h>
#include <Wire.h>

// Uncomment these to enable debugging features

//#define PN532DEBUG
//#define PN532_P2P_DEBUG


// lots of code has infinite timeouts; some dont. for redundancy, here it is
#define PN532_TIMEOUT 1000

// packet buffer size for sending/reading
#define PN532_PACKBUFFSZIE 80



/*
As per 6.2.1.1, an information frame (IF) is used to convey birectional communication
from the host controller to the PN532. The way this code is structued follows
the same format as IF defined in 6.2.1.1

0x00        0x00 0xFF       [LEN] [LCS] [TFI] [PD0] [PD1] ... [PDn]   [DCS]         0x00
Preamble    Start Code            [             DATA              ]              Postamble
Given this defintion, this means that the preamble, start code, and postamble can be defined
with macros.
*/

#define PN532_PREAMBLE                      (0x00) // Preamble,     start byte 1/3
#define PN532_STARTCODE1                    (0x00) // Start Code 1, start byte 2/3
#define PN532_STARTCODE2                    (0xFF) // Start Code 2, start byte 3/3
#define PN532_POSTAMBLE                     (0x00) // End of data

#define PN532_HOSTTOPN532                   (0xD4) // Host to PN532
#define PN532_PN532TOHOST                   (0xD5) // PN532 to Host

////////////////////////////////////////////////////////////////////////////
/*
quick notes:
 - wakeup can be done with GPIO R/W (0x06 & 0x08) and/or on IRQ, which is also a GPIO pin
 - unless security is a concern, SAM related commands can be ignored
    - might be useful in the future, however
 -
*/
// PN532 Commands
#define PN532_CMD_DIAGNOSE              (0x00) // Diagnose takes NumTst (1 byte) & InParam for some tests (7.2.1, pg 69 (nice))
#define PN532_CMD_GETFIRMWAREVERSION    (0x02) // Returns firmware in format IC - VER - REV - SUPPORT (7.2.3)
#define PN532_CMD_GETGENERALSTATUS      (0x04) // Lots of info. Check 7.2.3, lots of goodies for testing
#define PN532_CMD_READREGISTER          (0x06) // 7.2.4 
#define PN532_CMD_WRITEREGISTER         (0x08) // 7.2.5 
#define PN532_CMD_READGPIO              (0x0C) // 7.2.6
#define PN532_CMD_WRITEGPIO             (0x0E) // 7.2.7
#define PN532_CMD_SETSERIALBAUDRATE     (0x10) // ! - 7.2.8,  unused
#define PN532_CMD_SETPARAMETERS         (0x12) // 7.2.9, bit 3 must be 0, look into DID
#define PN532_CMD_SAMCONFIGURATION      (0x14) // ! - 7.2.10, unused, 0x01 default
#define PN532_CMD_POWERDOWN             (0x16)
#define PN532_CMD_RFCONFIGURATION       (0x32)
#define PN532_CMD_RFREGULATIONTEST      (0x58)
#define PN532_CMD_INJUMPFORDEP          (0x56)
#define PN532_CMD_INJUMPFORPSL          (0x46)
#define PN532_CMD_INLISTPASSIVETARGET   (0x4A)
#define PN532_CMD_INATR                 (0x50)
#define PN532_CMD_INPSL                 (0x4E)
#define PN532_CMD_INDATAEXCHANGE        (0x40)
#define PN532_CMD_INCOMMUNICATETHRU     (0x42)
#define PN532_CMD_INDESELECT            (0x44)
#define PN532_CMD_INRELEASE             (0x52)
#define PN532_CMD_INSELECT              (0x54)
#define PN532_CMD_INAUTOPOLL            (0x60)
#define PN532_CMD_TGINITASTARGET        (0x8C)
#define PN532_CMD_TGSETGENERALBYTES     (0x92)
#define PN532_CMD_TGGETDATA             (0x86)
#define PN532_CMD_TGSETDATA             (0x8E)
#define PN532_CMD_TGSETMETADATA         (0x94)
#define PN532_CMD_TGGETINITIATORCOMMAND (0x88)
#define PN532_CMD_TGRESPONSETOINITIATOR (0x90)
#define PN532_CMD_TGGETTARGETSTATUS     (0x8A)

#define PN532_WAKEUP                        (0x55)

#define PN532_SPI_STATREAD                  (0x02)
#define PN532_SPI_DATAWRITE                 (0x01)
#define PN532_SPI_DATAREAD                  (0x03)
#define PN532_SPI_READY                     (0x01)

#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READBIT                   (0x01)
#define PN532_I2C_BUSY                      (0x00)
#define PN532_I2C_READY                     (0x01)
#define PN532_I2C_READYTIMEOUT              (20)

#define NFC_WAIT_TIME                       30
#define NFC_CMD_BUF_LEN                     64
#define NFC_FRAME_ID_INDEX                  6

/*
todo:
 - function for constructing the standard information frame
    - data frame function to construct any given data to byte format
        - this probably means setting up a void pointer and checking data types
        - or, probably implementing it with Wire
 - multiple device chatter; send mask of 0x02 (bit 1) to 0x12 to specify DID usage
        - then, assign IDs to each module
 - extended frame builder - given a l
 - 
*/
class PN532 {
    public:
        PN532(uint8_t IRQ_, uint8_t RST_, TwoWire &Wire);
        void begin(); 
        void wakeup();
    protected:
        bool PN532Status(byte Status);
        bool CheckAck(byte *cmd, byte cmdlen);
        byte PN532_RD(byte *buff, byte len);
        bool PN532_RD_PACKET(byte *buff, byte len);
        bool PN532_WR_CMD(byte *cmd, byte len);
    private:
        TwoWire* WirePN532;
        int8_t IRQ = -1;
        int8_t RST = -1;
};