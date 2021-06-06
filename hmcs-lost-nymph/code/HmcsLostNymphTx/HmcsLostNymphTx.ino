#include <HmcsLostNymph.h>

#define BIND_BUTTON_HOLD_TIME     2000

#define LED_RED_OUT()  DDRB  |=  _BV(5)
#define LED_RED_ON()   PORTB |=  _BV(5)
#define LED_RED_OFF()  PORTB &= ~_BV(5)
#define LED_GRN_OUT()  DDRB  |=  _BV(4)
#define LED_GRN_ON()   PORTB |=  _BV(4)
#define LED_GRN_OFF()  PORTB &= ~_BV(4)

#define RFM_IRQ_PIN_SETUP() do { DDRD &= ~_BV(2); DDRD |= _BV(2); } while (0)
#define RFM_IRQ_ASSERTED()  ((PIND & _BV(2))==0x00)

#define BIND_BTN_SETUP()   do { DDRB &= ~_BV(3); PORTB |= _BV(3); } while (0)
#define BIND_BTN_PRESSED() ((PINB & _BV(3)) == 0)

#define PIN_BUZZER 10

uint8_t radio_statemach = RADIOSM_IDLE;

uint8_t hop_idx = 0;
bool need_bind = false;
uint32_t bind_uid = 0;
uint8_t mp_rxnum = 0;
uint8_t tx_rssi;

bool mp_ready = false;
bool telem_good = false;

uint32_t bind_start_time = 0;

void setup()
{
    Serial.begin(100000, SERIAL_8E2);
    LED_RED_OUT();
    LED_RED_OFF();
    LED_GRN_OUT();
    LED_GRN_OFF();
    nvm_load(0);

    RFM_IRQ_PIN_SETUP();
    radio_init();
    BIND_BTN_SETUP();

    while (millis() == 0) {
        // do nothing
    }
}

void loop()
{
    heartbeat_task();
    radio_task();
    telemetry_task();
    taranis_task();
    bind_task();
    #ifdef PIN_BUZZER
    buzz_task();
    #endif
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

#ifdef PIN_BUZZER

uint32_t buzz_start = 0;
uint32_t buzz_dur = 0;

void buzz_task()
{
    if (buzz_start != 0)
    {
        uint32_t now = millis();
        if ((now - buzz_start) >= buzz_dur)
        {
            #ifdef PIN_BUZZER
            digitalWrite(PIN_BUZZER, LOW);
            #endif
            buzz_start = 0;
            buzz_dur = 0;
        }
    }
}

void buzz(uint32_t dur)
{
    buzz_start = millis();
    buzz_dur = dur;
    #ifdef PIN_BUZZER
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, HIGH);
    #endif
}

#endif