#ifndef _MULPROT_DEFS_H_
#define _MULPROT_DEFS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define MULTIPROTOCOL_HEADER             0x55
#define MULTIPROTOCOL_PROTOCOL_OPENLRS   0x1B
#define MULTIPROTOCOL_TOTAL_CHANNELS     16
#define SBUS_BYTECNT                     22
#define PULSE_CENTER_US                  1500
#define PULSE_MAXDIFF_US                 512

typedef struct
{
    uint8_t header;
    uint8_t subprotocol;
    uint8_t rxnum_power_type;
    int8_t options;                 // stream[3]
    uint8_t chandata[SBUS_BYTECNT];
    uint8_t telemflags;             // stream[26]
    uint8_t additional[9];
}
multiprot_pkt_t;

struct sbus_help {
  uint16_t ch0 : 11;
  uint16_t ch1 : 11;
  uint16_t ch2 : 11;
  uint16_t ch3 : 11;
  uint16_t ch4 : 11;
  uint16_t ch5 : 11;
  uint16_t ch6 : 11;
  uint16_t ch7 : 11;
} __attribute__ ((__packed__));

struct sbus {
  struct sbus_help ch[2];
  uint8_t status;
}  __attribute__ ((__packed__));

typedef union ppm_msg {
  uint8_t  bytes[32];
  uint16_t words[16];
  struct sbus sbus;
} ppm_msg_t;

#ifdef __cplusplus
}
#endif

#endif