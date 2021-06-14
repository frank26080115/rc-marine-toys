#include <MPTX.h>

struct sbus_help {
  uint16_t ch0 : 11;
  uint16_t ch1 : 11;
  uint16_t ch2 : 11;
  uint16_t ch3 : 11;
  uint16_t ch4 : 11;
  uint16_t ch5 : 11;
  uint16_t ch6 : 11;
  uint16_t ch7 : 11;
} __attribute__ ((__packed__));

struct sbus {
  struct sbus_help ch[2];
  uint8_t status;
}  __attribute__ ((__packed__));

typedef union ppm_msg {
  uint8_t  bytes[32];
  uint16_t words[16];
  struct sbus sbus;
} ppm_msg_t;

void MPTX::begin(HardwareSerial* installSerial)
{
    myserial = installSerial;
    myserial->begin(100000, SERIAL_8E1_RXINV);
}

bool MPTX::rc_task(bool must)
{
    uint32_t now = millis();
    if ((now - lastTxTime) < 10 && must == false)
    {
        return false;
    }

    lastTxTime = now;

    if (bind == false)
    {
        myserial->write(0x55);
        myserial->write(subprotocol & 0x1F);
        fill_sbus((uint16_t*)channels);
    }
    else
    {
        myserial->write(0x57);
        myserial->write((subprotocol & 0x1F) | 0x20);
        fill_sbus((uint16_t*)failsafes);
    }

    myserial->write((rxnum & 0x0F) | ((subprotocoltype & 0x07) << 4));
    myserial->write(0);
    myserial->write(sbusBuffer, MPTX_SBUSBUFFER_SIZE);
    myserial->write(0);
    if (bind != false)
    {
        myserial->write(0);
    }
    return true;
}

void MPTX::fill_sbus(uint16_t* ch)
{
    ppm_msg_t* ppm = (ppm_msg_t*)sbusBuffer;
    for (uint8_t set = 0; set < 2; set++)
    {
        ppm->sbus.ch[set].ch0 = ch[(set << 3) + 0];
        ppm->sbus.ch[set].ch1 = ch[(set << 3) + 1];
        ppm->sbus.ch[set].ch2 = ch[(set << 3) + 2];
        ppm->sbus.ch[set].ch3 = ch[(set << 3) + 3];
        ppm->sbus.ch[set].ch4 = ch[(set << 3) + 4];
        ppm->sbus.ch[set].ch5 = ch[(set << 3) + 5];
        ppm->sbus.ch[set].ch6 = ch[(set << 3) + 6];
        ppm->sbus.ch[set].ch7 = ch[(set << 3) + 7];
    }
}

bool MPTX::telem_task()
{
    bool ret = false;
    uint8_t readcnt = 0;
    while (myserial->available() > 0 && readcnt < MPTX_TELEMRXBUFFER_SIZE)
    {
        uint8_t c = myserial->read();
        readcnt++;
        telemRxBuffer[telemRxIdx] = c;
        if (telemRxIdx == 0 && c != 'M')
        {
            telemRxLenCache = 0;
            //return false;
        }
        else if (telemRxIdx == 1 && c == 'M')
        {
            telemRxLenCache = 0;
            telemRxIdx = 1;
            //return false;
        }
        else if (telemRxIdx == 1 && c == 'P')
        {
            telemRxLenCache = 0;
            telemRxIdx = 2;
            //return false;
        }
        else if (telemRxIdx == 2)
        {
            telemRxLenCache = 0;
            telemRxIdx++;
            //return false;
        }
        else if (telemRxIdx == 3)
        {
            telemRxLenCache = c;
            telemRxIdx++;
            //return false;
        }
        else if (telemRxIdx >= 4 && telemRxLenCache > 0)
        {
            telemRxLenCache--;
            telemRxIdx++;
            if (telemRxLenCache <= 0)
            {
                if (telem_parse())
                {
                    lastTelemTime = millis();
                    telem_timeout = false;
                    ret = true;
                    telemRxIdx = 0;
                    //return true;
                }
            }
            if (telemRxIdx >= MPTX_TELEMRXBUFFER_SIZE)
            {
                telemRxLenCache = 0;
                telemRxIdx = c == 'M' ? 1 : 0;
                //return false;
            }
        }
        else
        {
            telemRxLenCache = 0;
            telemRxIdx = c == 'M' ? 1 : 0;
            //return false;
        }
    }
    if ((millis() - lastTelemTime) >= 2000)
    {
        telem_timeout = true;
    }
    return ret;
}

typedef struct
{
    uint8_t m;
    uint8_t p;
    uint8_t pkt_type;
    uint8_t pkt_len;
    uint16_t header;
    uint16_t sensor_id;
}
mptelem_t;

bool MPTX::telem_parse()
{
    mptelem_t* ptr = (mptelem_t*)telemRxBuffer;
    uint8_t* data = (uint8_t*)&(telemRxBuffer[sizeof(mptelem_t)]);
    if ((ptr->pkt_type != 0x02 && ptr->pkt_type != 0x03) || ptr->pkt_len < 9 || ptr->header != 0x1098)
    {
        return false;
    }
    switch (ptr->sensor_id)
    {
        case 0xF101:
            rssi_rx = data[0];
            rssi_tx = data[1];
            lqi_rx  = data[2];
            lqi_rx  = data[3];
            return true;
        case 0xF104:
            ain_batt = data[0];
            return true;
        case 0xF103:
            ain_2 = data[0];
            return true;
    }
    return false;
}
