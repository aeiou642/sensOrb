#ifdef NFC_INTERFACE_HSU

#include "PN532_HSU.h"
#include "PN532_debug.h"



#ifdef ESP32

PN532_HSU::PN532_HSU(HardwareSerial &serial, int8_t rxPin, int8_t txPin)
{
    hsu_serial = &serial;
    command = 0;
    _rxPin = rxPin;
    _txPin = txPin;
}
#else
#ifdef USE_SOFT_SERIAL_PIN
PN532_HSU::PN532_HSU(SoftwareSerial &serial){
    hsu_serial = &serial;
    command = 0;
}
#else
PN532_HSU::PN532_HSU(HardwareSerial &serial)
{
    hsu_serial = &serial;
    command = 0;
}
#endif
#endif

void PN532_HSU::begin()
{

#ifdef ESP32
    get_serial(hsu_serial)->begin(115200, SERIAL_8N1, _rxPin, _txPin);
#else
#ifdef USE_SOFT_SERIAL_PIN
    get_serial(hsu_serial)->begin(115200);
#else
    get_serial(hsu_serial)->begin(115200);
#endif
#endif
}

void PN532_HSU::wakeup()
{
    get_serial(hsu_serial)->write(0x55);
    get_serial(hsu_serial)->write(0x55);
    get_serial(hsu_serial)->write(uint8_t(0x00));
    get_serial(hsu_serial)->write(uint8_t(0x00));
    get_serial(hsu_serial)->write(uint8_t(0x00));

    /** dump serial buffer */
    if (get_serial(hsu_serial)->available())
    {
        DMSG("Dump serial buffer: ");
    }
    while (get_serial(hsu_serial)->available())
    {
        uint8_t ret = get_serial(hsu_serial)->read();
        DMSG_HEX(ret);
    }
}

int8_t PN532_HSU::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{

    /** dump serial buffer */
    if (get_serial(hsu_serial)->available())
    {
        DMSG("Dump serial buffer: ");
    }
    while (get_serial(hsu_serial)->available())
    {
        uint8_t ret = get_serial(hsu_serial)->read();
        DMSG_HEX(ret);
    }

    command = header[0];

    get_serial(hsu_serial)->write(uint8_t(PN532_PREAMBLE));
    get_serial(hsu_serial)->write(uint8_t(PN532_STARTCODE1));
    get_serial(hsu_serial)->write(uint8_t(PN532_STARTCODE2));

    uint8_t length = hlen + blen + 1; // length of data field: TFI + DATA
    get_serial(hsu_serial)->write(length);
    get_serial(hsu_serial)->write(~length + 1); // checksum of length

    get_serial(hsu_serial)->write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532; // sum of TFI + DATA

    DMSG("\nWrite: ");

    get_serial(hsu_serial)->write(header, hlen);
    for (uint8_t i = 0; i < hlen; i++)
    {
        sum += header[i];

        DMSG_HEX(header[i]);
    }

    get_serial(hsu_serial)->write(body, blen);
    for (uint8_t i = 0; i < blen; i++)
    {
        sum += body[i];

        DMSG_HEX(body[i]);
    }

    uint8_t checksum = ~sum + 1; // checksum of TFI + DATA
    get_serial(hsu_serial)->write(checksum);
    get_serial(hsu_serial)->write(uint8_t(PN532_POSTAMBLE));

    return readAckFrame();
}

int16_t PN532_HSU::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];

    DMSG("\nRead:  ");

    /** Frame Preamble and Start Code */
    if (receive(tmp, 3, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != tmp[0] || 0 != tmp[1] || 0xFF != tmp[2])
    {
        DMSG("Preamble error");
        return PN532_INVALID_FRAME;
    }

    /** receive length and check */
    uint8_t length[2];
    if (receive(length, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t)(length[0] + length[1]))
    {
        DMSG("Length error");
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if (length[0] > len)
    {
        return PN532_NO_SPACE;
    }

    /** receive command byte */
    uint8_t cmd = command + 1; // response command
    if (receive(tmp, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (PN532_PN532TOHOST != tmp[0] || cmd != tmp[1])
    {
        DMSG("Command error");
        return PN532_INVALID_FRAME;
    }

    if (receive(buf, length[0], timeout) != length[0])
    {
        return PN532_TIMEOUT;
    }
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for (uint8_t i = 0; i < length[0]; i++)
    {
        sum += buf[i];
    }

    /** checksum and postamble */
    if (receive(tmp, 2, timeout) <= 0)
    {
        return PN532_TIMEOUT;
    }
    if (0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1])
    {
        DMSG("Checksum error");
        return PN532_INVALID_FRAME;
    }

    return length[0];
}

int8_t PN532_HSU::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];

    DMSG("\nAck: ");

    if (receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0)
    {
        DMSG("Timeout\n");
        return PN532_TIMEOUT;
    }

    if (memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK)))
    {
        DMSG("Invalid\n");
        return PN532_INVALID_ACK;
    }
    return 0;
}

/**
    @brief receive data .
    @param buf --> return value buffer.
           len --> length expect to receive.
           timeout --> time of reveiving
    @retval number of received bytes, 0 means no data received.
*/
int8_t PN532_HSU::receive(uint8_t *buf, int len, uint16_t timeout)
{
    int read_bytes = 0;
    int ret;
    unsigned long start_millis;

    while (read_bytes < len)
    {
        start_millis = millis();
        do
        {
            ret = get_serial(hsu_serial)->read();
            if (ret >= 0)
            {
                break;
            }
        } while ((timeout == 0) || ((millis() - start_millis) < timeout));

        if (ret < 0)
        {
            if (read_bytes)
            {
                return read_bytes;
            }
            else
            {
                return PN532_TIMEOUT;
            }
            buf[read_bytes] = (uint8_t)ret;
            DMSG_HEX(ret);
            read_bytes++;
        }
        buf[read_bytes] = (uint8_t)ret;
        DMSG_HEX(ret);
        read_bytes++;
    }
    return read_bytes;
}

#endif // NFC_INTERFACE_HSU