#ifndef _HLN_CONFIG_H_
#define _HLN_CONFIG_H_

#include <stdint.h>

#define MAGIC_KEY   0xDEADBEAF
#define PRIVATE_KEY 0x2EF85CA8

//#define HLN_SEND_COMPRESSED

#define RFM_NOMINAL_FREQ     435000000
//#define RFM_NOMINAL_FREQ     240000000
#define RFM_FREQHOP_STEPSIZE 3
#define RFM_FREQHOP_CHANCNT  8
#define RFM_BIND_CHANNEL     1
#define RFM_TX_POWER         0x07 // maximum

#define PKT_INTVAL_MS        50

#define CHANHOP_NEXT_MS       (PKT_INTVAL_MS - (PKT_INTVAL_MS / 5))  // when to hop on successful reception, must be very slightly shorter than the normal interval
#define CHANHOP_MISSED_MS     (PKT_INTVAL_MS * 2)   // when to hop when a packet is missed, must be much longer than the normal interval

#define AS_FAST_AS_POSSIBLE

#define TELEM_LOST_MS        1000
#define TARANIS_TIMEOUT      1000
#define TARANIS_CONSOLE_TIMEOUT 5000

#define RC_USED_CHANNELS     8
#define RC_DEADZONE_US       8

#ifdef HLN_SEND_COMPRESSED
#define uid_t       uint16_t
#else
#define uid_t       uint32_t
#endif

#endif
