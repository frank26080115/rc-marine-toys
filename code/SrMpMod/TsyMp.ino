#ifdef TSYMP

#define DEADZONE 16

#ifndef PIN_LED
#define PIN_LED 13
#endif

#include <MPTX.h>

HardwareSerial* xiaoPort = &Serial1;
HardwareSerial* mpPort = &Serial2;
MPTX mptx;
xiaopkt_t xiaoPacket;

uint32_t sum_throttle = 0;
uint32_t sum_steering = 0;
uint32_t cal_cnt = 0;
double center_throttle = 0;
double center_steering = 0;
int32_t min_throttle = 128;
int32_t max_throttle = 128;
int32_t min_steering = 128;
int32_t max_steering = 128;

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  Serial.begin(115200);
  Serial.println("hello");
  disp_init();
  xiaoPort->begin(XIAO_BAUD);
  mptx.begin(mpPort);
  disp_show(false);
  while (millis() == 0)
  {
    
  }
}

void loop()
{
  static uint32_t last_xiao_time = 0;
  static uint32_t last_dbg_time = 0;

  bool dbg = false;
  static bool dbgmode = false;
  uint32_t now = millis();

  if (Serial.available() > 0)
  {
    uint8_t c = Serial.read();
    if (c == 'D')
    {
      dbgmode = true;
      last_dbg_time = now;
    }
    else if (c == 'C')
    {
      Serial.printf("Calibration: %d %d\r\n", lround(center_throttle), lround(center_steering));
      Serial.printf("ADC Limits: %d %d ; %d %d\r\n", min_throttle, max_throttle, min_steering, max_steering);
    }
    else if (c == 'A')
    {
      dump_xiao();
    }
  }

  if ((now - last_dbg_time) >= 250)
  {
    dbg = true;
    last_dbg_time = now;
  }

  if (center_throttle == 0 || center_steering == 0)
  {
    if (now < 2000)
    {
      xiao_task();
    }
    else if (cal_cnt < 64)
    {
      if (xiao_task())
      {
        sum_throttle += xiaoPacket.adc[ADCIDX_THROTTLE];
        sum_steering += xiaoPacket.adc[ADCIDX_STEERING];
        cal_cnt++;
        last_xiao_time = now;
      }
    }
    else
    {
      center_throttle = sum_throttle;
      center_steering = sum_steering;
      center_throttle /= (double)cal_cnt;
      center_steering /= (double)cal_cnt;
      min_throttle = 16;
      max_throttle = 230;
      min_steering = 32;
      max_steering = 240;
    }
    return;
  }

  if (xiao_task())
  {
    if (xiaoPacket.adc[ADCIDX_THROTTLE] < min_throttle)
    {
      min_throttle--;
      min_throttle -= (min_throttle - xiaoPacket.adc[ADCIDX_THROTTLE]) / 2;
    }
    if (xiaoPacket.adc[ADCIDX_THROTTLE] > max_throttle)
    {
      max_throttle++;
      max_throttle += (xiaoPacket.adc[ADCIDX_THROTTLE] - max_throttle) / 2;
    }
    if (xiaoPacket.adc[ADCIDX_STEERING] < min_steering)
    {
      min_steering--;
      min_steering -= (min_steering - xiaoPacket.adc[ADCIDX_STEERING]) / 2;
    }
    if (xiaoPacket.adc[ADCIDX_STEERING] > max_steering)
    {
      max_steering++;
      max_steering += (xiaoPacket.adc[ADCIDX_STEERING] - max_steering) / 2;
    }
    last_xiao_time = now;

    double th = calc_axis(xiaoPacket.adc[ADCIDX_THROTTLE], center_throttle, min_throttle, max_throttle, 128, DEADZONE);
    double st = calc_axis(xiaoPacket.adc[ADCIDX_STEERING], center_steering, min_steering, max_steering, 128, DEADZONE) * -1;

    if ((xiaoPacket.btn_flags & (1 << BITIDX_THREV)) != 0)
    {
      th *= -1;
    }
    if ((xiaoPacket.btn_flags & (1 << BITIDX_STREV)) != 0)
    {
      st *= -1;
    }

    digitalWrite(PIN_LED, (th != 0 || st != 0));

    int16_t left, right;
    double trim_th = calc_axis(xiaoPacket.adc[ADCIDX_TRIM_TH], 128, 0, 255, 256, DEADZONE) + 256;
    double trim_st = calc_axis(xiaoPacket.adc[ADCIDX_TRIM_ST], 128, 0, 255, 256, DEADZONE);
    int16_t trim_r = (int16_t)xiaoPacket.adc[ADCIDX_STDR];
    calc_tankmix(th, st, trim_th, trim_st, trim_r, &left, &right);
    disp_submit_rc(th, st, left, right, trim_th, trim_st, trim_r);

    if ((xiaoPacket.btn_flags & (1 << BITIDX_BIND)) != 0)
    {
      mptx.bind = true;
      mptx.failsafes[0] = 0;
      mptx.failsafes[1] = 0;
    }
    else
    {
      mptx.bind = false;
      mptx.channels[0] = 1500 + left;
      mptx.channels[1] = 1500 + right;
    }

    mptx.rc_task(false);
  }

  if ((now - last_xiao_time) >= 500)
  {
    last_xiao_time = now;
    mptx.channels[0] = 1500;
    mptx.channels[1] = 1500;
    mptx.rc_task(true);
    Serial.println("ERROR: internal data link timed out");
  }

  if (mptx.telem_task())
  {
    disp_submit_telem(mptx.ain_batt, mptx.rssi_rx, mptx.rssi_tx);
  }
  else
  {
    if (mptx.telem_timeout)
    {
      disp_submit_telem(0, 0, 0);
    }
  }

  disp_task(dbg || dbgmode);
}

double calc_axis(uint16_t adc, double center, uint16_t minv, uint16_t maxv, uint16_t limit, uint16_t deadzone)
{
  double x = adc;
  double range;
  if (x > (center - deadzone) && x < (center + deadzone))
  {
    return 0;
  }
  if (x >= (center + deadzone))
  {
    range = (maxv - deadzone) - (center + deadzone);
    x -= (center + deadzone);
    x *= limit;
    x /= range;
    x = x > limit ? limit : x;
    return x;
  }
  else // if (x <= (center - deadzone))
  {
    range = (center - deadzone) - (minv + deadzone);
    x = (center - deadzone) - x;
    x *= limit;
    x /= range;
    x = x > limit ? limit : x;
    return -x;
  }
}

void calc_tankmix(double th, double st, double trim_th, double trim_st, uint16_t trim_r, int16_t* out_left, int16_t* out_right)
{
  static int16_t old_left = 0;
  static int16_t old_right = 0;
  double left = th + st;
  double right = th - st;

  left = left > 128 ? 128 : (left < -128 ? -128 : left);
  right = right > 128 ? 128 : (right < -128 ? -128 : right);

  left *= trim_th;
  left /= 256;
  right *= trim_th;
  right /= 256;

  if ((left > 0 && right > 0) || (left < 0 && right < 0))
  {
    if (trim_st > 0)
    {
      left *= 256 - trim_st;
      left /= 256;
    }
    else if (trim_st < 0)
    {
      right *= 256 + trim_st;
      right /= 256;
    }
  }

  int16_t lefti  = (int16_t)lround(left  > 128 ? 128 : (left  < -128 ? -128 : left ));
  int16_t righti = (int16_t)lround(right > 128 ? 128 : (right < -128 ? -128 : right));

  if (lefti > old_left)
  {
    old_left += trim_r;
    old_left = old_left > lefti ? lefti : old_left;
  }
  else if (lefti < old_left)
  {
    old_left -= trim_r;
    old_left = old_left < lefti ? lefti : old_left;
  }

  if (righti > old_right)
  {
    old_right += trim_r;
    old_right = old_right > righti ? righti : old_right;
  }
  else if (righti < old_right)
  {
    old_right -= trim_r;
    old_right = old_right < righti ? righti : old_right;
  }

  *out_left  = old_left * 4;
  *out_right = old_right * 4;
}


bool xiao_task()
{
  static uint8_t buff[sizeof(xiaopkt_t)];
  static uint8_t buffidx = 0;
  xiaopkt_t* ptr = (xiaopkt_t*)buff;
  uint8_t readcnt = 0;
  bool ret = false;
  while (xiaoPort->available() > 0
    //&& readcnt < 64
    )
  {
    uint8_t c = xiaoPort->read();
    readcnt++;
    buff[buffidx] = c;
    if (buffidx == 0 && c == MAGIC_1)
    {
      buffidx++;
    }
    else if (buffidx == 1 && c == MAGIC_1)
    {
      buffidx = 1;
    }
    else if (buffidx == 1 && c == MAGIC_2)
    {
      buffidx++;
    }
    else if (buffidx < (sizeof(xiaopkt_t) - 2))
    {
      buffidx++;
    }
    else if (buffidx < sizeof(xiaopkt_t))
    {
      buffidx++;
      if (buffidx == sizeof(xiaopkt_t))
      {
        uint16_t chksum = calc_chksum(buff);
        if (chksum == ptr->chksum)
        //if (true)
        {
          memcpy(&xiaoPacket, buff, sizeof(xiaopkt_t));
          ret = true;
          buffidx = 0;
        }
        else
        {
          buffidx = c == MAGIC_1 ? 1 : 0;
        }
      }
    }
    else
    {
      buffidx = c == MAGIC_1 ? 1 : 0;
    }
  }
  return ret;
}

void dump_xiao()
{
  Serial.print("DUMP XIAO: ");
  for (int i = 0; i < sizeof(xiaopkt_t); i++)
  {
    Serial.printf("0x%02X ", ((uint8_t*)(&xiaoPacket))[i]);
  }
  Serial.println();
  Serial.send_now();
}

#endif
