#ifdef TSYMP

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <MPTX.h>

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define SCREEN_ADDRESS      0x3C
#define SCREEN_ROW_SPACE    8
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

extern MPTX mptx;

bool disp_hasRc = false;
bool disp_hasTelem = false;
double disp_th, disp_st, disp_trimth, disp_trimst;
int16_t disp_left, disp_right, disp_trimr;
int16_t disp_batt, disp_rssirx, disp_rssitx;

void disp_submit_rc(double th, double st, int16_t left, int16_t right, double trim_th, double trim_st, int16_t trim_r)
{
  disp_hasRc = true;
  disp_th = th;
  disp_st = st;
  disp_left = left;
  disp_right = right;
  disp_trimth = trim_th;
  disp_trimst = trim_st;
  disp_trimr = trim_r;
}

void disp_submit_telem(int16_t batt, int16_t rssi_rx, int16_t rssi_tx)
{
  disp_hasTelem = true;
}

void disp_task(bool dbg)
{
  static uint32_t frame_time = 0;
  uint32_t now = millis();
  if ((now - frame_time) < 100)
  {
    return;
  }
  frame_time = now;
  disp_show(dbg);
}

void disp_show(bool dbg)
{
  #define STRBUF_LEN 64
  static char strbuf[STRBUF_LEN + 2];
  int len;
  int row = 0;
  display.clearDisplay();
  display.stopscroll();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, row * SCREEN_ROW_SPACE);
  display.cp437(true); // use full 256 char 'Code Page 437' font

  if (dbg) { Serial.print("> "); }

  if (disp_hasTelem)
  {
    snprintf(strbuf, STRBUF_LEN, "BATT %d", disp_batt);
    //snprintf(strbuf, STRBUF_LEN, "BATT %3.1f", disp_batt);
    display.print(strbuf);
    if (dbg) { Serial.print(strbuf); Serial.print(" ; "); }
    row++;
    display.setCursor(0, row * 8);
    snprintf(strbuf, STRBUF_LEN, "RSSI %d  %d", disp_rssirx, disp_rssitx);
    display.print(strbuf);
    if (dbg) { Serial.print(strbuf); Serial.print(" ; "); }
    row++;
    display.setCursor(0, row * SCREEN_ROW_SPACE);
  }
  else
  {
    display.print("NO TELEM");
    row++;
    display.setCursor(0, row * SCREEN_ROW_SPACE);
  }

  if (disp_hasRc)
  {
    snprintf(strbuf, STRBUF_LEN, "TH %d  ST %d", lround(disp_th), lround(disp_st));
    if (dbg) { Serial.print(strbuf); Serial.print(" ; "); }
    display.print(strbuf);
    row++;
    display.setCursor(0, row * 8);
    snprintf(strbuf, STRBUF_LEN, "L %d   R %d", disp_left, disp_right);
    if (dbg) { Serial.print(strbuf); Serial.print(" ; "); }
    display.print(strbuf);
    row++;
    display.setCursor(0, row * SCREEN_ROW_SPACE);
    len = snprintf(strbuf, STRBUF_LEN, "TR %d %d %d", lround((disp_trimst * 100.0) / 256.0), lround((disp_trimth * 100.0) / 256.0), lround((((double)disp_trimr) * 100.0) / 255.0));
    //if (len > 16) {
    //  snprintf(strbuf, STRBUF_LEN, "T %d %d %d", lround(disp_trimst), lround(disp_trimth), disp_trimr);
    //}
    if (dbg) { Serial.print(strbuf); Serial.print(" ; "); }
    display.print(strbuf);
    row++;
    display.setCursor(0, row * SCREEN_ROW_SPACE);
  }
  else
  {
    display.print("NO RC");
    row++;
    display.setCursor(0, row * SCREEN_ROW_SPACE);
  }

  if (mptx.bind)
  {
    display.print("BIND");
    if (dbg) { Serial.print("BIND"); Serial.print(" ; "); }
    row++;
    display.setCursor(0, row * SCREEN_ROW_SPACE);
  }

  display.display();
  if (dbg) { Serial.println(); Serial.send_now(); }
}

void disp_init()
{
  //SSD1306_SWITCHCAPVCC
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, false, true) == false)
  {
    while (1)
    {
      Serial.println("failed OLED display init");
      delay(1000);
    }
  }
}

void disp_test()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 1);
  display.cp437(true); // use full 256 char 'Code Page 437' font
  display.print("test");
  display.display();
  while (1);
}

#endif
