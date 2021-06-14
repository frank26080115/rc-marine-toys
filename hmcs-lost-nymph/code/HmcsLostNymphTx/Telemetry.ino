#define TELEMODE_MULPRO 0
#define TELEMODE_SPORT  1
#define TELEMODE_FRSKYD 2
#define TELEMETRY_MODE  TELEMODE_SPORT

uint8_t telem_schedule  = 0;
uint32_t telem_sendTime = 0;
uint32_t telem_rxTime   = 0;

uint8_t telem_a1        = 0;
uint8_t telem_a2        = 0;
uint8_t telem_rssi_rx   = 0;
uint8_t telem_rssi_tx   = 0;
#ifdef USE_LQI
uint8_t telem_lqi_rx    = 0;
uint8_t telem_lqi_tx    = 0;
#endif
uint8_t telem_profile   = 0;

#if TELEMETRY_MODE == TELEMODE_MULPRO || TELEMETRY_MODE == TELEMODE_SPORT

#define TELEM_INTERVAL_MS 12
#define TELEM_BAUD        100000
// I think because of the circuitry and capabilities of the OrangeTx LRS module, TELEMODE_SPORT is the only mode possible

void smartport_send(uint8_t* data)
{
    uint16_t crc = 0;
    #if TELEMETRY_MODE == TELEMODE_MULPRO
    Serial.write('M');
    Serial.write('P');
    Serial.write(0x02); // type = S.PORT telemetry
    Serial.write(9);    // length, excluding the header
    #endif
    #if TELEMETRY_MODE == TELEMODE_SPORT
    Serial.write(0x7E);
    #endif
    for (int i = 0; i < 9; i++)
    {
        if (i == 8)
        {
            data[i] = 0xFF - crc;
        }
        #if TELEMETRY_MODE == TELEMODE_SPORT
        if ((data[i] == 0x7E) || (data[i] == 0x7D))
        {
            Serial.write(0x7D);
            Serial.write(0x20 ^ data[i]);
        }
        else
        #endif
        {
            Serial.write(data[i]);
        }
        if (i > 0)
        {
            crc += data[i];     //0-1FF
            crc += crc >> 8; //0-100
            crc &= 0x00FF;
        }
    }
}

void smartport_send_idle()
{
    #if TELEMETRY_MODE == TELEMODE_SPORT
    Serial.write(0x7E);
    #endif
}

#elif TELEMETRY_MODE == TELEMODE_FRSKYD

#define TELEM_INTERVAL_MS 30
#define TELEM_BAUD        57600

void frskyd_sendStuffed(uint8_t* data)
{
    Serial.write(0x7E);
    for (uint8_t i = 0; i < 9; i++)
    {
        if ((data[i] == 0x7E) || (data[i] == 0x7D))
        {
            Serial.write(0x7D);
            data[i] ^= 0x20;
        }
        Serial.write(data[i]);
    }
    Serial.write(0x7E);
}

#endif

void report_telemetry(uint8_t a1, uint8_t a2, uint8_t rssi_rx, uint8_t rssi_tx
    #ifdef USE_LQI
        , uint8_t rssi_rx, uint8_t rssi_tx
    #endif
        , uint8_tuint8_t profile)
{
    telem_rxTime = millis();
    telem_a1 = a1;
    telem_a2 = a2;
    telem_rssi_rx = rssi_rx;
    telem_rssi_tx = rssi_tx;
    #ifdef USE_LQI
    telem_lqi_rx = lqi_rx;
    telem_lqi_tx = lqi_tx;
    #endif
    telem_profile = profile;
    telemetry_task();
}

void telemetry_task()
{
#ifdef TELEMETRY_MODE
    uint32_t now = millis();
    if ((now - telem_rxTime) >= TELEM_LOST_MS)
    {
        telem_good = false;
        return;
    }
    if ((now - telem_sendTime) < TELEM_INTERVAL_MS)
    {
        return;
    }
    telem_sendTime = now;
    telem_good = true;
#if TELEMETRY_MODE == TELEMODE_MULPRO || TELEMETRY_MODE == TELEMODE_SPORT
    uint8_t buf[9];
    buf[0] = 0x98;
    buf[1] = 0x10;
    telem_schedule = (telem_schedule + 1) % 36;
    switch (telem_schedule) {
        case 0: // SWR
            buf[2] = 0x05;
            buf[3] = 0xF1;
            buf[4] = telem_profile + 1;
            break;
        case 1: // RSSI
            buf[2] = 0x01;
            buf[3] = 0xF1;
            buf[4] = (uint16_t)(telem_rssi_rx) * 100 / 256;
            buf[5] = (uint16_t)(telem_rssi_tx) * 100 / 256;
            #ifdef USE_LQI
            buf[6] = telem_lqi_rx;
            buf[7] = telem_lqi_tx;
            #endif
            break;
        case 2: //BATT
            buf[2] = 0x04;
            buf[3] = 0xF1;
            buf[4] = telem_a1;
            break;
        case 3: //A2
            buf[2] = 0x03;
            buf[3] = 0xF1;
            buf[4] = telem_a2;
            break;
        default:
            smartport_send_idle();
            return;
    }
    smartport_send(buf);
    //telem_lastTime = millis();
#elif TELEMETRY_MODE == TELEMODE_FRSKYD
    uint8_t frame[9];
    if (telem_schedule == 0)
    {
        frame[0] = 0xFE;
        frame[1] = telem_a1;
        frame[2] = telem_a2;
        frame[3] = (telem_rssi_rx >> 1); // this needs to be 0-127
        frame[4] = telem_rssi_tx;        // this needs to be 0-255
        frame[5] = frame[6] = frame[7] = frame[8] = 0;
        frskyd_sendStuffed(frame);
    }
    telem_schedule = (telem_schedule + 1) % 6;
#else
#endif
#endif
}