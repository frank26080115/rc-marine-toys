#include <HmcsLostNymph.h>

#define LED_RED_OUT()  DDRB  |=  _BV(5);
#define LED_RED_ON()   PORTB |=  _BV(5);
#define LED_RED_OFF()  PORTB &= ~_BV(5);
#define LED_GRN_OUT()  DDRB  |=  _BV(4);
#define LED_GRN_ON()   PORTB |=  _BV(4);
#define LED_GRN_OFF()  PORTB &= ~_BV(4);

#define RFM_IRQ_PIN_SETUP() do { DDRD &= ~_BV(2); DDRD |= _BV(2); } while (0)
#define RFM_IRQ_ASSERTED()  ((PIND & 0x04)==0x00)

uint8_t run_mode = RUNMODE_TX_START;

uint8_t hop_idx = 0;
bool need_bind = false;
uint32_t bind_uid = 0;

bool mp_ready = false;
bool telem_good = false;

void setup()
{
    Serial.begin(100000, SERIAL_8E2);
    LED_RED_OUT();
    LED_RED_OFF();
    LED_GRN_OUT();
    LED_GRN_OFF();
    RFM_IRQ_PIN_SETUP();
    nvm_read();

    radio_init();
}

void radio_init()
{
    // TODO: init SPI here

    rfmSetReadyMode(); // turn on XTAL
    delayMicroseconds(600); // time to settle
    rfmClearIntStatus();
    rfmInit(0);
    rfmSetStepSize(RFM_FREQHOP_STEPSIZE);

    uint32_t magic = MAGIC_KEY;
    for (uint8_t i = 0; i < 4; i++)
    {
        rfmSetHeader(i, (magic >> 24) & 0xFF);
        magic = magic << 8; // advance to next byte
    }

    rfmSetModemRegs(&modem_params[RFM_DEFAULT_PARAM_IDX]);
    rfmSetPower(RFM_TX_POWER);
    rfmSetCarrierFrequency(RFM_NOMINAL_FREQ);
}

void loop()
{
    heartbeat_task();
    radio_task();
    telemetry_task();
    taranis_task();
    bind_task();
}

void heartbeat_task()
{
    uint32_t now = millis();
    uint32_t dur = 100; // blink duration, which is different depending on the status

    if (need_bind != false)
    {
        return;
    }

    if (mp_ready != false)
    {
        dur += 350;
    }

    if (telem_good != false)
    {
        LED_RED_OFF();
    }
    else
    {
        LED_RED_ON();
    }

    if (channels_has_movement() != false)
    {
        dur += 350;
    }

    if ((now % 1000) < dur)
    {
        LED_GRN_ON();
    }
    else
    {
        LED_GRN_OFF();
    }
}
