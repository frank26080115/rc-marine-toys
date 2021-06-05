#include <HmcsLostNymph.h>

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

uint8_t radio_statemach = RADIOSM_IDLE;
bool link_good = false;
bool debug_enabled = false;

uint16_t telem_afcc;

void setup()
{
    Serial.begin(57600);
    LED_RED_OUT();
    LED_RED_OFF();
    LED_GRN_OUT();
    LED_GRN_OFF();
    motors_init();
    nvm_read();
    RFM_IRQ_PIN_SETUP();
    radio_init();
    BIND_BTN_SETUP();

    while (millis() == 0) {
        // do nothing
    }
}

void loop()
{
    radio_task();
    //motor_task();
    bind_task();
    heartbeat_task();
    cli_task();
}

void radio_task()
{
    static uint32_t last_hop_time = 0;
    static uint32_t last_tx_time = 0;
    static uint32_t next_hop_delay = 0;
    static uint32_t missed_pkts = 0;
    uint32_t now = millis();

    switch (radio_statemach)
    {
        case RADIOSM_IDLE:
            hop_idx = 0;
            rfmSetChannel(get_hop_chan(hop_idx));
            radio_statemach = RADIOSM_RX_START;
            last_hop_time = now;
            break;
        case RADIOSM_RX_WAIT:
            if (RFM_IRQ_ASSERTED() != false)
            {
                int8_t rx_len;
                uint16_t afcc;
                rfmClearIntStatus();
                rx_rssi = rfmGetRSSI();
                telem_afcc = rfmGetAFCC();

                rx_len = rfmGetWholePacket(rfm_buffer, RFM_PKT_LEN);

                if (debug_enabled)
                {
                    Serial.printf("RX %d $d %d ", rx_len, rx_rssi, telem_afcc);
                }

                pkthdr_t* hp = (pkthdr_t*)rfm_buffer;
                if (need_bind == false)
                {
                    if (check_packet(rfm_buffer, nvm.tx_uid, nvm.rx_uid) != false && hp->pkt_type == PKTTYPE_REMOTECONTROL)
                    {
                        #ifdef HLN_SEND_COMPRESSED
                        uint8_t bytes2read = (((RC_USED_CHANNELS + 1) * 11) / 8);
                        memcpy(sbus_buffer, (void*)(&(rfm_buffer[sizeof(pkthdr_t)])), bytes2read
                        decode_sbus((uint8_t*)sbus_buffer, (uint16_t*)channel);
                        #else
                        memcpy(channel, (void*)(&(rfm_buffer[sizeof(pkthdr_t)])), RC_USED_CHANNELS * sizeof(uint16_t));
                        #endif

                        if (debug_enabled)
                        {
                            Serial.printf("C %d %d %d ", channel[0], channel[1], channel[2]);
                        }

                        last_hop_time = now;
                        next_hop_delay = CHANHOP_NEXT_MS;
                        link_good = true;
                        missed_pkts = 0;
                        motor_task();
                        radio_statemach = RADIOSM_TX_START;
                    }
                }
                else
                {
                    // bind
                    if (check_packet(rfm_buffer, 0, 0) != false && hp->pkt_type == PKTTYPE_BIND)
                    {
                        nvm.tx_uid = hp->tx_uid;
                        nvm.rx_uid = gen_rand_uid();

                        if (debug_enabled)
                        {
                            Serial.printf("B 0x%08X 0x%08X ", nvm.tx_uid, nvm.rx_uid);
                        }

                        nvm_save(0);
                        last_hop_time = now;
                        missed_pkts = 0;
                        next_hop_delay = CHANHOP_NEXT_MS;
                        radio_statemach = RADIOSM_BIND_TX;
                    }
                }

                if (debug_enabled)
                {
                    Seria.println();
                }
            }
            else if (need_bind != false)
            {
                if ((now - last_hop_time) > PKT_INTVAL_MS)
                {
                    radio_statemach = RADIOSM_BIND_START;
                }
            }
            else
            {
                if ((now - last_hop_time) > next_hop_delay)
                {
                    radio_statemach = RADIOSM_RX_HOP;
                    if (next_hop_delay == CHANHOP_MISSED_MS)
                    {
                        missed_pkts++;
                        if (missed_pkts >= MISSED_PKTS_THRESH)
                        {
                            missed_pkts = MISSED_PKTS_THRESH;
                            link_good = false;
                            failsafe();
                            radio_statemach = RADIOSM_RX_START; // no more hopping, no point
                        }
                    }
                    next_hop_delay = CHANHOP_MISSED_MS;
                }
            }
            break;
        case RADIOSM_RX_HOP:
            hop_idx = (hop_idx + 1) % (sizeof(uid_t) * 2);
            rfmSetChannel(get_hop_chan(hop_idx));
            radio_statemach = RADIOSM_RX_START;
            last_hop_time = now;
            // fall through
        case RADIOSM_RX_START:
            /*
            if (need_bind)
            {
                rfmSetChannel(RFM_BIND_CHANNEL);
                last_hop_time = now;
            }
            //*/
            rfmClearIntStatus();
            rfmClearFIFO(0);
            rfmSetRX();
            radio_statemach = RADIOSM_RX_WAIT;
            break;
        case RADIOSM_TX_START:
            build_packet(nvm.tx_uid, nvm.rx_uid, rfm_buffer, PKTTYPE_TELEMETRY);
            uint8_t data_idx = sizeof(pkthdr_t);
            telemetry_fill();
            data_idx += sizeof(telem_pkt_t);
            rfmSendPacket(rfm_buffer, data_idx);
            rfmClearIntStatus();
            rfmSetTX();
            radio_statemach = RADIOSM_TX_WAIT;
            // fall through
        case RADIOSM_TX_WAIT:
            if (RFM_IRQ_ASSERTED() == false)
            {
                // no interrupt pin assertion
                if ((now - last_tx_time) >= PKT_INTVAL_MS)
                {
                    // timeout waiting for TX
                    radio_statemach = need_bind == false ? RADIOSM_RX_START : RADIOSM_BIND_START;
                }
            }
            else
            {
                // yes interrupt pin assertion
                radio_statemach = need_bind == false ? RADIOSM_RX_START : RADIOSM_BIND_START;
            }
            break;
        case RADIOSM_BIND_START:
            rfmSetChannel(RFM_BIND_CHANNEL);
            radio_statemach = RADIOSM_RX_START;
            last_hop_time = now;
            break;
        case RADIOSM_BIND_TX:
            rfmSetChannel(RFM_BIND_CHANNEL);
            build_packet(nvm.tx_uid, nvm.rx_uid, rfm_buffer, PKTTYPE_BIND);
            uint8_t data_idx = sizeof(pkthdr_t);
            rfmSendPacket(rfm_buffer, data_idx);
            rfmClearIntStatus();
            rfmSetTX();
            last_tx_time = now;
            radio_statemach = RADIOSM_BIND_WAIT;
            break;
        case RADIOSM_BIND_WAIT:
            bool next = false;
            if (RFM_IRQ_ASSERTED() != false)
            {
                rfmClearIntStatus();
                if ((now - last_tx_time) >= PKT_INTVAL_MS)
                {
                    next = true;
                }
            }
            else
            {
                next = true;
            }
            if (next != false)
            {
                if (need_bind == false)
                {
                    hop_idx = 0;
                    rfmSetChannel(get_hop_chan(hop_idx));
                    last_hop_time = now;
                    radio_statemach = RADIOSM_RX_START;
                }
                else
                {
                    radio_statemach = RADIOSM_BIND_TX;
                }
            }
            break;
    }
}

void bind_task()
{
    static bool prev_need_bind = false;
    static bool prev_btn = false;
    static uint32_t btn_time = 0;
    uint32_t now = millis();

    if (BIND_BUTTON_PRESSED() != false)
    {
        if (prev_btn == false)
        {
            btn_time = now;
        }
        else
        {
            if ((now - btn_time) > BIND_BUTTON_HOLD_TIME)
            {
                if (debug_enabled)
                {
                    Serial.println("BIND BTN");
                }
                need_bind = true;
            }
        }
        prev_btn = true;
    }
    else
    {
        if (prev_btn != false)
        {
            if (debug_enabled)
            {
                Serial.println("BIND BTN released");
            }
        }

        prev_btn = false;
        need_bind = false;
    }

    if (prev_need_bind != false && need_bind == false)
    {
        LED_RED_OFF();
        LED_GRN_OFF();
    }
    else if (need_bind != false)
    {
        if (radio_statemach == RADIOSM_BIND_TX || radio_statemach == RADIOSM_BIND_WAIT)
        {
            LED_RED_OFF();
            if (((now / 100) % 2) == 0)
            {
                LED_GRN_ON();
            }
            else
            {
                LED_GRN_OFF();
            }
        }
        else
        {
            if (((now / 200) % 2) == 0)
            {
                LED_GRN_ON();
                LED_RED_OFF();
            }
            else
            {
                LED_GRN_OFF();
                LED_RED_ON();
            }
        }
    }
}

void heartbeat_task()
{
    uint32_t now = millis();
    uint32_t dur = 100; // blink duration, which is different depending on the status

    if (need_bind != false)
    {
        // bind_task takes care of blinking
        return;
    }

    if (link_good != false)
    {
        dur += 350;
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

void telemetry_fill()
{
    telem_pkt_t* tp = (telem_pkt_t*)&(rfm_buffer[sizeof(pkthdr_t)]);
    tp->a1 = analogRead(ADCPIN_BATT) >> 2;
    tp->a2 = analogRead(ADCPIN_AIN2) >> 2;
    tp->rssi = rx_rssi;
    if (debug_enabled)
    {
        Serial.printf("T %d %d %d", tp->a1, tp->a2, tp->rssi);
        Serial.println();
    }
}
