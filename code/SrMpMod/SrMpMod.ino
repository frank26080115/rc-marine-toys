//#define XIAO
#define TSYMP

typedef struct
{
  uint8_t magic_1;
  uint8_t magic_2;
  uint8_t adc[5];
  uint8_t btn_flags;
  uint16_t chksum;
}
xiaopkt_t;

#define XIAO_BAUD 9600

#define MAGIC_1 0xAA
#define MAGIC_2 0x5A

#define ADCIDX_THROTTLE 0
#define ADCIDX_STEERING 1
#define ADCIDX_TRIM_TH  2
#define ADCIDX_TRIM_ST  3
#define ADCIDX_STDR     4
#define BITIDX_BIND     0
#define BITIDX_STREV    1
#define BITIDX_THREV    2

uint16_t calc_chksum(void* ptr)
{
  uint8_t* data = (uint8_t*)ptr;
  uint8_t count = sizeof(xiaopkt_t) - sizeof(uint16_t);
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  uint8_t index;
  for ( index = 0; index < count; ++index )
  {
    sum1 = (sum1 + data[index]) % 255;
    sum2 = (sum2 + sum1) % 255;
  }
  return (sum2 << 8) | sum1;
}
