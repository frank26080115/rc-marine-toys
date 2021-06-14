#include "aadc.h"
#include <Arduino.h>
#include <math.h>

typedef struct
{
    bool     valid;
    uint8_t  ard_pin;
    uint32_t SC1A;
    double   filter;
    double   result;
    uint16_t raw;
    bool     new_flag;
}
AADC_chan_t;

enum
{
    AADCSM_INIT,
    AADCSM_START,
    AADCSM_WAIT,
};

/*
static const uint8_t pin2sc1a[] = {
	5, 14, 8, 9, 13, 12, 6, 7, 15, 11, 0, 4+64, 23, // 0-12 -> A0-A12
	255, // 13 is digital only (no A13 alias)
	5, 14, 8, 9, 13, 12, 6, 7, 15, 11, 0, 4+64, 23, // 14-26 are A0-A12
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, // 27-37 unused
	26, // 38=temperature
	27  // 39=bandgap ref (PMC_REGSC |= PMC_REGSC_BGBE)
};
*/

static AADC_chan_t aadc_channels[] = {
    { true , 0, 0, 0.1, -1, false }, // battery
    { true , 0, 0, 0.1, -1, false }, // internal pressure
    { true , 0, 0, 0.1, -1, false }, // external pressure
    { false, 0, 0, 0  , -1, false },
};

static uint8_t state_machine = AADCSM_INIT;
static uint8_t chan_idx = 0;

void AADC_done(uint32_t res);

void AADC_task()
{
    uint32_t scla;
    switch (state_machine)
    {
        case AADCSM_INIT:
            chan_idx = 0;
            analogReadRes(12);
            analogReadAveraging(0);
            analogRead(aadc_channels[0].ard_pin);
            state_machine = AADCSM_START;
            break;
        case AADCSM_START:
            scla = aadc_channels[chan_idx].SC1A;
            #if defined(__MKL26Z64__)
            if (scla & 0x40) {
                ADC0_CFG2 &= ~ADC_CFG2_MUXSEL;
                scla &= 0x3F;
            } else {
                ADC0_CFG2 |= ADC_CFG2_MUXSEL;
            }
            #endif
            ADC0_SC1A = scla;
            state_machine = AADCSM_WAIT;
            break;
        case AADCSM_WAIT:
            if ((ADC0_SC1A & ADC_SC1_COCO) != 0) {
                AADC_done(ADC0_RA);
            }
            break;
    }
}

void AADC_done(uint32_t res)
{
    AADC_chan_t* p = &aadc_channels[chan_idx];
    p->raw = res;

    if (p->result < 0 || p->filter == 0)
    {
        p->result = res;
        p->new_flag = true;
    }
    else
    {
        double old_res = p->result;
        double new_res = res;
        double next_res = (new_res * p->filter) + (old_res * (1 - p->filter));
        p->result = next_res;
        p->new_flag = true;
    }

    chan_idx++;
    if (aadc_channels[chan_idx].valid == false)
    {
        chan_idx = 0;
    }
    state_machine = AADCSM_START;
}

bool AADC_hasNew(uint8_t c)
{
    return aadc_channels[c].new_flag;
}

void AADC_clearNew(uint8_t c)
{
    aadc_channels[c].new_flag = false;
}

double AADC_read(uint8_t c)
{
    return aadc_channels[c].result;
}

uint16_t AADC_read16(uint8_t c)
{
    return (uint16_t)lround(aadc_channels[c].result);
}

uint16_t AADC_readRaw(uint8_t c)
{
    return aadc_channels[c].raw;
}
