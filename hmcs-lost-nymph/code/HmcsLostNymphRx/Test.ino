void test_rfecho(bool sweep)
{
    uint8_t ch = RFM_BIND_CHANNEL;
    while (1)
    {
        uint32_t now = millis();
        rfmSetChannel(ch);
        rfmClearIntStatus();
        rfmClearFIFO(0);
        rfmSetRX();
        Serial.printf("RX millis %d\r\n", now);
        if (sweep)
        {
            Serial.printf("chan %d\r\n", ch);
        }
        uint32_t t = now;
        while (RFM_IRQ_ASSERTED() == false)
        {
            if (((now = millis()) - t) > 1000)
            {
                Serial.print(".");
                Serial.send_now();
            }
        }
        Serial.println("!");
        Serial.println("IRQ = received");
        Serial.printf("RSSI %d\r\n", rfmGetRSSI());
        Serial.printf("AFCC %d\r\n", rfmGetAFCC());
        int rx_len = rfmGetWholePacket(rfm_buffer, RFM_PKT_LEN);
        Serial.printf("DATA[%d]: ", rx_len);
        for (int i = 0; i < rx_len; i++)
        {
            Serial.printf("0x%02X ", rfm_buffer[i]);
        }
        Serial.println("done");

        Serial.println("TX echo");
        rfmSendPacket(rfm_buffer, rx_len);
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
        Serial.println("!");
        Serial.println("IRQ = sent");
        if (sweep)
        {
            ch++;
            if (ch >= 255)
            {
                ch = 1;
            }
        }
    }
}

void test_io()
{
    while(1)
    {
        if (BIND_BUTTON_PRESSED())
        {
            Serial.println("BIND");
            LED_GRN_ON();
            LED_RED_OFF();
        }
        else
        {
            Serial.println(analogRead(ADCPIN_BATT));
            LED_GRN_OFF();
            LED_RED_ON();
        }
        delay(100);
    }
}

void test_motors()
{
    while (1)
    {
        motor_buzzAll();
        for (int m = 0; m < 3; m++)
        {
            for (uint16_t p = PULSE_CENTER_US; p <= PULSE_CENTER_US + PULSE_MAXDIFF_US; p += 32)
            {
                channel[m] = p;
                motor_task();
                delay(100);
            }
            for (uint16_t p = PULSE_CENTER_US + PULSE_MAXDIFF_US; p >= PULSE_CENTER_US - PULSE_MAXDIFF_US; p -= 32)
            {
                channel[m] = p;
                motor_task();
                delay(100);
            }
            for (uint16_t p = PULSE_CENTER_US - PULSE_MAXDIFF_US; p <= PULSE_CENTER_US; p += 32)
            {
                channel[m] = p;
                motor_task();
                delay(100);
            }
            channel[m] = PULSE_CENTER_US + PULSE_MAXDIFF_US;
            motor_task();
            delay(2000);
            channel[m] = PULSE_CENTER_US;
            for (int i = 0; i < 2000; i++)
            {
                motor_task();
                delay(1);
            }
        }
    }
}