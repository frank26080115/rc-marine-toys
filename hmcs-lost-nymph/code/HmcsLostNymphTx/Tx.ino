void radio_task()
{
    static uint32_t last_tx_time = 0;

    uint32_t now = millis();

    switch (run_mode)
    {
        case RUNMODE_RX_WAIT:
            if ((now - last_tx_time) < PKT_INTVAL_MS)
            {
                if (rx has data)
                {
                    tx_rssi = rfmGetRSSI();
                    rfmGetPacket(rfm_buffer, TELEMETRY_PACKETSIZE);
                    run_mode = RUNMODE_RX_START;
                    pkthdr_t* p = (pkthdr_t*)rfm_buffer;
                    if (need_bind != false && check_packet(rfm_buffer, bind_uid, 0) != false && p->pkt_type == PKTTYPE_BIND)
                    {
                        nvm.tx_uid = bind_uid;
                        nvm.rx_uid = p->rx_uid;
                        nvm_save();
                        run_mode = RUNMODE_BIND_FINISHED;
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
            run_mode = need_bind == false ? RUNMODE_TX_START : RUNMODE_BIND_START;
        case RUNMODE_TX_START:
            last_tx_time = now;
            rfmSetChannel(get_hop_chan(hop_idx));
            build_packet(nvm.rx_uid, nvm.tx_uid, rfm_buffer, PKTTYPE_REMOTECONTROL);
            uint8_t data_idx = sizeof(pkthdr_t);
            uint8_t i;
            for (i = 0; i < RC_USED_CHANNELS; i++)
            {
                uint8_t* ptr = (uint8_t*)&(channel[i]);
                rfm_buffer[data_idx] = ptr[0];
                data_idx++;
                rfm_buffer[data_idx] = ptr[1];
                data_idx++;
            }
            rfmSendPacket(rfm_buffer, data_idx);
            rfmClearIntStatus();
            rfmSetTX();
            run_mode = RUNMODE_TX_WAIT;
            hop_idx = (hop_idx + 1) % 8;
            break;
        case RUNMODE_BIND_START:
            last_tx_time = now;
            hop_idx = 0;
            rfmSetChannel(RFM_BIND_CHANNEL);
            build_packet(0, bind_uid, rfm_buffer, PKTTYPE_BIND);
            uint8_t data_idx = sizeof(pkthdr_t);
            rfmSendPacket(rfm_buffer, data_idx);
            rfmClearIntStatus();
            rfmSetTX();
            run_mode = RUNMODE_TX_WAIT;
            break;
        case RUNMODE_TX_WAIT:
            if (interrupt didn't happen)
            {
                if ((now - last_tx_time) >= PKT_INTVAL_MS)
                {
                    // timeout waiting for TX
                    run_mode = RUNMODE_TX_START;
                    break;
                }
            }
            rfmClearIntStatus();
            run_mode = RUNMODE_RX_START;
        case RUNMODE_RX_START:
            rfmClearFIFO(0);
            rfmClearIntStatus();
            rfmSetRX();
            run_mode = RUNMODE_RX_WAIT;
            break;
        case RUNMODE_BIND_FINISHED:
            if (need_bind == false)
            {
                run_mode = RUNMODE_RX_WAIT;
            }
            break;
    }
}

void taranis_task()
{
    static uint8_t pkt_idx = 0;
    static uint8_t mp_buffer[32];
    static multiprot_pkt_t* mp_pkt = (multiprot_pkt_t*)mp_buffer;
    static uint32_t last_mp_time = 0;

    uint32_t now = millis();

    if (last_mp_time == 0 || (now - last_mp_time) > TARANIS_TIMEOUT)
    {
        mp_ready = false;
        center_all_channels();
        last_mp_time = now;
    }

    if (Serial.available() > 0)
    {
        while (Serial.available() > 0)
        {
            uint8_t c = Serial.read();
            mp_buffer[pkt_idx] = c;
            if (pkt_idx == 0)
            {
                if (mp_pkt->header == MULTIPROTOCOL_HEADER)
                {
                    pkt_idx++;
                }
            }
            else if (pkt_idx == 1)
            {
                if ((mp_pkt->subprotocol & 0x1F) == MULTIPROTOCOL_PROTOCOL_OPENLRS)
                {
                    pkt_idx++;
                }
                else
                {
                    pkt_idx = 0;
                }
                need_bind = ((mp_pkt->subprotocol & 0x80) != 0);
            }
            else if (pkt_idx <= 3)
            {
                pkt_idx++; // don't care about these bytes
            }
            else if (pkt_idx <= 25)
            {
                if (pkt_idx == 25) // we have all of the pulse data
                {
                    decode_sbus((uint8_t*)&(mp_buffer[4]), (uint16_t*)channel);
                    last_mp_time = now;
                    mp_ready = true;
                    pkt_idx = 0; // ignore rest of packet
                }
                else
                {
                    pkt_idx++;
                }
            }
        }
    }
}

void bind_task()
{
    static bool prev_need_bind = false;
    static bool prev_btn = false;
    static uint32_t btn_time = 0;
    uint32_t now = millis();

    if (digitalRead(PIN_BUTTON) == LOW)
    {
        if (prev_btn == false)
        {
            srand(micros() ^ millis());
            btn_time = now;
        }
        else
        {
            if ((now - btn_time) > BIND_BUTTON_HOLD_TIME)
            {
                need_bind = true;
            }
        }
        prev_btn = true;
    }
    else
    {
        prev_btn = false;
    }

    if (prev_need_bind == false && need_bind != false)
    {
        srand(micros() ^ millis());
        do
        {
            bind_uid = rand();
        }
        while (bind_uid == 0);
    }
    prev_need_bind = need_bind;

    if (prev_need_bind != false && need_bind == false)
    {
        LED_RED_OFF();
        LED_GRN_OFF();
    }
    else if (need_bind != false)
    {
        if (run_mode == RUNMODE_BIND_FINISHED)
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

uint16_t multiproto_2_pulseUs(uint16_t x)
{
    int32_t value = x;
    // from https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/Multiprotocol.h
    value = (((value - 204) * 1000) / 1639) + 1000;
    return value
}

void decode_sbus(uint8_t* packet, uint16_t* channels)
{
    ppm_msg_t* ptr = (ppm_msg_t*)(pkt);
    uint8_t set;
    for (set = 0; set < 2; set++)
    {
        channel[(set << 3) + 0] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch0);
        channel[(set << 3) + 1] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch1);
        channel[(set << 3) + 2] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch2);
        channel[(set << 3) + 3] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch3);
        channel[(set << 3) + 4] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch4);
        channel[(set << 3) + 5] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch5);
        channel[(set << 3) + 6] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch6);
        channel[(set << 3) + 7] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch7);
    }
}
