void test_rftx_cont()
{
    while (1)
    {
        uint32_t now = millis();
        rfmSetChannel(RFM_BIND_CHANNEL);
        rfmClearIntStatus();
        rfmClearFIFO(0);
        rfmSendPacket((uint8_t*)(&now), sizeof(uint32_t));
        rfmClearIntStatus();
        rfmSetTX();

        now = millis();
        t = now;
        while (RFM_IRQ_ASSERTED() == false)
        {
            if (((now = millis()) - t) > 1000)
            {
                Serial.print(".");
                Serial.send_now();
            }
        }
        Serial.println(F("!"));
        Serial.println(F("IRQ = sent"));
    }
}

void test_rftx_sweep(uint32_t dly)
{
    uint8_t ch = 1;
    while (1)
    {
        uint32_t now = millis();
        rfmSetChannel(ch);
        rfmClearIntStatus();
        rfmClearFIFO(0);
        rfmSendPacket((uint8_t*)(&now), sizeof(uint32_t));
        rfmClearIntStatus();
        rfmSetTX();
        Serial.print(F("chan "));
        Serial.print(ch, DEC);
        Serial.print(F(" millis "));
        Serial.print(now, DEC);

        now = millis();
        t = now;
        while (RFM_IRQ_ASSERTED() == false)
        {
            if (((now = millis()) - t) > 1000)
            {
                Serial.print(".");
                Serial.send_now();
            }
        }
        Serial.println(F("!"));
        Serial.println(F("IRQ = sent"));
        ch++;
        if (ch >= 255)
        {
            ch = 1;
        }
        delay(dly);
    }
}

void test_io()
{
    while(1)
    {
        if (BIND_BUTTON_PRESSED())
        {
            Serial.println(F("BIND"));
            LED_GRN_ON();
            LED_RED_OFF();
        }
        else
        {
            LED_GRN_OFF();
            LED_RED_ON();
        }
        delay(100);
    }
}

void test_multiprotocol()
{
    while (1)
    {
        if (taranis_task() != false)
        {
            Serial.print(F("> RXNUM "));
            Serial.print(mp_rxnum, DEC);
            Serial.print(F(" SUB 0x"));
            Serial.print(mp_pkt->subprotocol, HEX);
            Serial.print(F(" CHDATA "));
            for (int i = 0; i < 4; i++)
            {
                Serial.print(channel[i], DEC);
                Serial.print(F(" "));
            }
            Serial.println();
        }
    }
}