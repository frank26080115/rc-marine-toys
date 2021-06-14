#define PIN_ADC_THROTTLE 1
#define PIN_ADC_STEERING 0
#define PIN_ADC_TRIM_TH  3
#define PIN_ADC_TRIM_ST  4
#define PIN_ADC_STDR     2
#define PIN_SW_BIND      5
#define PIN_SW_STREV     9
#define PIN_SW_THREV     8

#ifdef XIAO

xiaopkt_t pkt;

void send_packet()
{
  pkt.magic_1 = MAGIC_1;
  pkt.magic_2 = MAGIC_2;
  pkt.adc[ADCIDX_THROTTLE] = analogRead(PIN_ADC_THROTTLE) >> 2;
  pkt.adc[ADCIDX_STEERING] = analogRead(PIN_ADC_STEERING) >> 2;
  pkt.adc[ADCIDX_TRIM_TH]  = analogRead(PIN_ADC_TRIM_TH)  >> 2;
  pkt.adc[ADCIDX_TRIM_ST]  = analogRead(PIN_ADC_TRIM_ST)  >> 2;
  pkt.adc[ADCIDX_STDR]     = analogRead(PIN_ADC_STDR)     >> 2;
  pkt.btn_flags = 0
                    | ((digitalRead(PIN_SW_BIND)  == LOW ? 1 : 0) << BITIDX_BIND)
                    | ((digitalRead(PIN_SW_STREV) == LOW ? 0 : 1) << BITIDX_STREV)
                    | ((digitalRead(PIN_SW_THREV) == LOW ? 0 : 1) << BITIDX_THREV);
  pkt.chksum = calc_chksum(&pkt);
  Serial1.write((uint8_t*)&pkt, sizeof(pkt));
}

void setup()
{
  pinMode(PIN_SW_BIND,  INPUT);
  pinMode(PIN_SW_STREV, INPUT);
  pinMode(PIN_SW_THREV, INPUT);
  Serial1.begin(XIAO_BAUD
      //, SERIAL_8N2
      );
  Serial.begin(115200);
  Serial.println("hello world");
  while (millis() == 0) {
    
  }
}

void loop()
{
  static uint32_t last_time = 0;
  static uint32_t last_dbg_time = 0;
  uint32_t now = millis();
  //if ((now - last_time) >= 10)
  {
    send_packet();
    delay(1);
    last_time = now;
  }
  if ((now - last_dbg_time) >= 500 && last_dbg_time != 0)
  {
    dbg_pkt();
    last_dbg_time = now;
  }
  if (Serial.available() > 0)
  {
    uint8_t c = Serial.read();
    if (c == 'D')
    {
      dbg_pkt();
      last_dbg_time = now;
    }
  }
}

void dbg_pkt()
{
  Serial.print("> ");
  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(pkt.adc[i], DEC);
    Serial.print(" ");
  }
  Serial.println(pkt.btn_flags, BIN);
}

#endif
