#ifndef _HLN_CONFIG_H_
#define _HLN_CONFIG_H_

#define MAGIC_KEY   0xDEADBEAF
#define PRIVATE_KEY 0x2EF85CA8

//#define RFM_NOMINAL_FREQ     435000000
#define RFM_NOMINAL_FREQ     240000000
#define RFM_FREQHOP_STEPSIZE 3
#define RFM_FREQHOP_CHANCNT  8
#define RFM_BIND_CHANNEL     1
#define RFM_TX_POWER         0x07 // maximum

#define PKT_INTVAL_MS        50

#define PKT_NEXTHOP_MS       40  // when to hop on successful reception, must be very slightly shorter than the normal interval
#define PKT_MISSHOP_MS       100 // when to hop when a packet is missed, must be much longer than the normal interval

#define TELEM_LOST_MS        1000
#define TARANIS_TIMEOUT      1000

#define RC_USED_CHANNELS     3
#define RC_MINIMUM_MOVE      10

#endif
