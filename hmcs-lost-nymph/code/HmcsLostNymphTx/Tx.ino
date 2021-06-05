uint16_t local_afcc;//

void radio_task()
{
    static uint32_t last_tx_time = 0;

    uint32_t now = millis();

    switch (radio_statemach)
    {
        case RADIOSM_IDLE:
            radio_statemach = need_bind == false ? RADIOSM_TX_START : RADIOSM_BIND_START;
            break;
        case RADIOSM_RX_WAIT:
            if ((now - last_tx_time) < PKT_INTVAL_MS)
            {
                if (RFM_IRQ_ASSERTED() != false)
                {
                    int8_t rx_len;
                    tx_rssi = rfmGetRSSI();
                    //local_afcc = rfmGetAFCC();
                    rx_len = rfmGetWholePacket(rfm_buffer, RFM_PKT_LEN);
                    radio_statemach = RADIOSM_RX_START;
                    pkthdr_t* p = (pkthdr_t*)rfm_buffer;
                    if (need_bind != false && check_packet(rfm_buffer, bind_uid, 0) != false && p->pkt_type == PKTTYPE_BIND)
                    {
                        #ifdef PIN_BUZZER
                        buzz(1000);
                        #endif
                        nvm.tx_uid = bind_uid;
                        nvm.rx_uid = p->rx_uid;
                        nvm_save(mp_rxnum);
                        radio_statemach = RADIOSM_BIND_WAIT;
                    }
                    else if (check_packet(rfm_buffer, nvm.tx_uid, nvm.rx_uid) != false)
                    {
                        if (p->pkt_type == PKTTYPE_TELEMETRY)
                        {
                            telem_pkt_t* tp = (telem_pkt_t*)&(rfm_buffer[sizeof(pkthdr_t)]);
                            report_telemetry(tp->a1, tp->a2, tp->rssi, tx_rssi, 0);
                        }
                    }
                }
                break;
            }
            // timeout
            radio_statemach = need_bind == false ? RADIOSM_TX_START : RADIOSM_BIND_START;
            // fall through
        case RADIOSM_TX_START:
            last_tx_time = now;
            rfmSetChannel(get_hop_chan(hop_idx));
            build_packet(nvm.tx_uid, nvm.rx_uid, rfm_buffer, PKTTYPE_REMOTECONTROL);
            uint8_t data_idx = sizeof(pkthdr_t);

            #ifdef HLN_SEND_COMPRESSED
            uint32_t bytes2send = (((RC_USED_CHANNELS + 1) * 11) / 8);
            bytes2send = (bytes2send > SBUS_BYTECNT) ? SBUS_BYTECNT : bytes2send;
            for (uint8_t i = 0; i < bytes2send; i++)
            {
                rfm_buffer[data_idx] = ((uint8_t*)sbus_buff)[i];
                data_idx++;
            }
            #else
            for (uint8_t i = 0; i < RC_USED_CHANNELS; i++)
            {
                uint8_t* ptr = (uint8_t*)&(channel[i]);
                rfm_buffer[data_idx] = ptr[0];
                data_idx++;
                rfm_buffer[data_idx] = ptr[1];
                data_idx++;
            }
            #endif

            rfmSendPacket(rfm_buffer, data_idx);
            rfmClearIntStatus();
            rfmSetTX();
            radio_statemach = RADIOSM_TX_WAIT;
            hop_idx = (hop_idx + 1) % (sizeof(uid_t) * 2);
            break;
        case RADIOSM_BIND_START:
            last_tx_time = now;
            hop_idx = 0;
            rfmSetChannel(RFM_BIND_CHANNEL);
            build_packet(bind_uid, 0, rfm_buffer, PKTTYPE_BIND);
            uint8_t data_idx = sizeof(pkthdr_t);
            rfmSendPacket(rfm_buffer, data_idx);
            rfmClearIntStatus();
            rfmSetTX();
            radio_statemach = RADIOSM_TX_WAIT;
            break;
        case RADIOSM_TX_WAIT:
            if (RFM_IRQ_ASSERTED() == false)
            {
                // no interrupt pin assertion
                if ((now - last_tx_time) >= PKT_INTVAL_MS)
                {
                    // timeout waiting for TX
                    radio_statemach = RADIOSM_TX_START;
                }
                break;
            }
            else
            {
                // yes interrupt pin assertion
                rfmClearIntStatus();
                radio_statemach = RADIOSM_RX_START;
                // fall through
            }
        case RADIOSM_RX_START:
            rfmClearFIFO(0);
            rfmClearIntStatus();
            rfmSetRX();
            radio_statemach = RADIOSM_RX_WAIT;
            break;
        case RADIOSM_BIND_WAIT:
            if (need_bind == false)
            {
                radio_statemach = RADIOSM_RX_WAIT;
            }
            break;
    }
}

#define MP_BUFFER_SIZE 32
uint8_t mp_buffer[MP_BUFFER_SIZE];
multiprot_pkt_t* mp_pkt;

bool taranis_task()
{
    static uint8_t pkt_idx = 0;
    static uint8_t prev_rxnum = 0;
    static uint32_t last_mp_time = 0;

    bool ret = false;

    uint32_t now = millis();

    if (last_mp_time == 0 || (now - last_mp_time) > TARANIS_TIMEOUT)
    {
        mp_pkt = (multiprot_pkt_t*)mp_buffer;
        mp_ready = false;
        center_all_channels();
    }

    #if TARANIS_CONSOLE_TIMEOUT > 0
    if ((now - last_mp_time) > TARANIS_CONSOLE_TIMEOUT)
    {
        Serial.begin(57600, SERIAL_8N2);
        cli_task();
        return;
    }
    #endif

    //if (Serial.available() > 0)
    {
        while (Serial.available() > 0 && (millis() - now) <= 2)
        {
            uint8_t c = Serial.read();
            mp_buffer[pkt_idx] = c;
            if (pkt_idx == 0)
            {
                if (mp_pkt->header == MULTIPROTOCOL_HEADER)
                {
                    pkt_idx++;
                    #ifdef DEBUG_MULTIPROTOCOL_ECHO
                    Serial.write(c);
                    #endif
                }
            }
            else if (pkt_idx == 1)
            {
                if ((mp_pkt->subprotocol & 0x1F) == MULTIPROTOCOL_PROTOCOL_OPENLRS)
                {
                    if ((mp_pkt->subprotocol & 0x80) != 0)
                    {
                        need_bind = true;
                        bind_start_time = now;
                    }
                    pkt_idx++;
                    #ifdef DEBUG_MULTIPROTOCOL_ECHO
                    Serial.write(c);
                    #endif
                }
                else if (c == MULTIPROTOCOL_HEADER)
                {
                    // another header, hmm... pretend we just sync'ed on header
                    pkt_idx = 1;
                }
                else
                {
                    pkt_idx = 0; // error, sync on header again
                }
            }
            else if (pkt_idx <= 3)
            {
                pkt_idx++; // don't care about these bytes
                #ifdef DEBUG_MULTIPROTOCOL_ECHO
                Serial.write(c);
                #endif
            }
            else if (pkt_idx <= 25)
            {
                if (pkt_idx == 25) // we have all of the pulse data
                {
                    ret = true;
                    #ifdef HLN_SEND_COMPRESSED
                    memcpy((uint8_t*)sbus_buff, (uint8_t*)&(mp_buffer[4]), SBUS_BYTECNT);
                    #endif
                    decode_sbus((uint8_t*)&(mp_buffer[4]), (uint16_t*)channel);
                    last_mp_time = now;
                    mp_ready = true;
                    mp_rxnum = mp_pkt->rxnum_power_type & 0x0F;
                    if (prev_rxnum != mp_rxnum)
                    {
                        nvm_load(mp_rxnum);
                        prev_rxnum = mp_rxnum;
                    }
                    pkt_idx = 0; // ignore rest of packet
                    // obtaining a false header byte in the next byte will not cause a malfunction
                    // bytes 27 to 35 will never come if the Taranis is configured correctly
                }
                else
                {
                    pkt_idx++;
                    #ifdef DEBUG_MULTIPROTOCOL_ECHO
                    Serial.write(c);
                    #endif
                }
            }
        }
    }
    return ret;
}

uint32_t bind_start_time = 0;

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
                prev_need_bind = false; // this tricks the next bit of code to regenerate the random UID
                need_bind = true;
                bind_start_time = now;
            }
        }
        prev_btn = true;
    }
    else
    {
        prev_btn = false;
        if (bind_start_time != 0)
        {
            if ((now - bind_start_time) > 500)
            {
                need_bind = false;
                bind_start_time = 0;
            }
        }
    }

    if (prev_need_bind == false && need_bind != false)
    {
        bind_uid = gen_rand_uid();
    }
    prev_need_bind = need_bind;

    if (prev_need_bind != false && need_bind == false)
    {
        LED_RED_OFF();
        LED_GRN_OFF();
        bind_start_time = 0;
    }
    else if (need_bind != false)
    {
        if (radio_statemach == RADIOSM_BIND_FINISHED)
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
