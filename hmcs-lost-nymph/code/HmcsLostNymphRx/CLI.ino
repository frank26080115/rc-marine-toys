void cli_task()
{
    //if (Serial.available() > 0)
    {
        uint8_t readcnt = 0;
        while (Serial.available() > 0 && readcnt < 8)
        {
            uint8_t c = Serial.read();
            readcnt++;
            if (c == 'd')
            {
                debug_enabled = true;
            }
        }
    }
}
