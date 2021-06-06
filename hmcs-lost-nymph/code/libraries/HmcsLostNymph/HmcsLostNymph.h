#ifndef _HMCSLOSTNYMPH_H_
#define _HMCSLOSTNYMPH_H_

#include <Arduino.h>
#include <stdint.h>

#include "hln_config.h"
#include "rfm.h"
#include "multiprotocol_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uid_t tx_uid;
    uid_t rx_uid;
}
nvm_t;

enum
{
    PKTTYPE_BIND,
    PKTTYPE_REMOTECONTROL,
    PKTTYPE_TELEMETRY,
};

enum
{
    RADIOSM_IDLE,
    RADIOSM_RX_START,
    RADIOSM_RX_WAIT,
    RADIOSM_RX_HOP,
    RADIOSM_TX_START,
    RADIOSM_TX_WAIT,
    RADIOSM_BIND_START,
    RADIOSM_BIND_TX,
    RADIOSM_BIND_WAIT,
};

typedef struct
{
    #ifdef EXPLICIT_MAGIC
    uid_t magic_key;
    #endif
    uid_t tx_uid;
    uid_t rx_uid;
    uid_t hdr_chk;
    uint8_t  pkt_type;
}
pkthdr_t;

typedef struct
{
    uint8_t a1;
    uint8_t a2;
    uint8_t rssi;
    #ifdef EXTENDED_TELEMETRY
    uint16_t afcc;
    uint8_t good_pkts;
    #endif
}
telem_pkt_t;

extern uint16_t channel[MULTIPROTOCOL_TOTAL_CHANNELS];
extern uint8_t  rfm_buffer[RFM_PKT_LEN];
#ifdef HLN_SEND_COMPRESSED
extern uint8_t  sbus_buffer[RFM_PKT_LEN];
#endif
extern nvm_t nvm;
extern rfm22_modem_regs_t modem_params[RFM_AVAILABLE_PARAMS];

uint8_t get_hop_chan(uint8_t idx);
uid_t calc_hdrchk(uid_t tx_uid, uid_t rx_uid);
void build_packet(uid_t tx_uid, uid_t rx_uid, uint8_t* buffer, uint8_t pkt_type);
bool check_packet(uint8_t* buffer, uid_t tx_uid, uid_t rx_uid);
void center_all_channels();
bool channels_has_movement();
uid_t gen_rand_uid();
void nvm_save(uint8_t idx);
void nvm_load(uint8_t idx);
void radio_init();
uint16_t multiproto_2_pulseUs(uint16_t x);
void decode_sbus(uint8_t* packet, uint16_t* channels);


#ifdef __cplusplus
}
#endif

#endif
