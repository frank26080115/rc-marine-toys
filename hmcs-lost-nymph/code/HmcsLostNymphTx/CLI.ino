void cli_task()
{
    //if (Serial.available() > 0)
    {
        uint8_t readcnt = 0;
        while (Serial.available() > 0 && readcnt < 8)
        {
            uint8_t c = Serial.read();
            readcnt++;
            if (c == 's')
            {
                simulate_enabled = true;
            }
            else if (c == 'S')
            {
                simulate_enabled = false;
            }
        }
    }

    if (simulate_enabled != false)
    {
        simulate_task();
    }
}

void simulate_task()
{
    uint32_t now = millis();
    for (uint8_t c = 0; c < MULTIPROTOCOL_TOTAL_CHANNELS; c++)
    {
        uint32_t x = now % (PULSE_MAXDIFF_US * 2);
        uint32_t offset = c * 100;
        x += offset + PULSE_CENTER_US;
        while (x > PULSE_CENTER_US + PULSE_MAXDIFF_US) {
            x -= PULSE_MAXDIFF_US;
        }
        channel[c] = x;
    }
}