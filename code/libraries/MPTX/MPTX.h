#ifndef _MPTX_H_
#define _MPTX_H_

#include <stdint.h>
#include <Arduino.h>
#include <HardwareSerial.h>

#define MPTX_RCCHANNEL_CNT 16
#define MPTX_TELEMRXBUFFER_SIZE 64
#define MPTX_SBUSBUFFER_SIZE 22

class MPTX
{
    public:
        uint16_t channels [MPTX_RCCHANNEL_CNT]; // microsecond units, 1500 is center
        uint16_t failsafes[MPTX_RCCHANNEL_CNT]; // 0 = no pulse, 2047 = hold last pulse
        uint8_t  subprotocol     = 0;           // FrSkyX     = 15
        uint8_t  subprotocoltype = 0;           // FrSkyX CH8 = 1
        bool     bind            = false;
        uint8_t  rxnum           = 0;
        bool     telem_timeout   = true;

        uint8_t rssi_rx  = 0;
        uint8_t rssi_tx  = 0;
        uint8_t lqi_rx   = 0;
        uint8_t lqi_tx   = 0;
        uint8_t ain_batt = 0;
        uint8_t ain_2    = 0;

        void begin(HardwareSerial*);
        bool rc_task(bool);
        bool telem_task();
    private:
        HardwareSerial* myserial;
        uint8_t  telemRxBuffer[MPTX_TELEMRXBUFFER_SIZE];
        uint8_t  sbusBuffer   [MPTX_SBUSBUFFER_SIZE];
        uint8_t  telemRxIdx      = 0;
        uint8_t  telemRxLenCache = 0;
        uint32_t lastTxTime      = 0;
        uint32_t lastTelemTime   = 0;
        bool     telem_parse();
        void     fill_sbus(uint16_t*);
};

#endif
