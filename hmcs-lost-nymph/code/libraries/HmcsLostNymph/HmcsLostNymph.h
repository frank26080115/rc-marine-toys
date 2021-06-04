#ifndef _HMCSLOSTNYMPH_H_
#define _HMCSLOSTNYMPH_H_

#include "hln_config.h"
#include "rfm.h"
#include "multiprotocol_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t tx_uid;
    uint32_t rx_uid;
}
nvm_t;

enum
{
    PKTTYPE_BIND_TX,
    PKTTYPE_BIND_RX,
    PKTTYPE_REMOTECONTROL,
    PKTTYPE_TELEMETRY,
    PKTTYPE_TX_AUX,
    PKTTYPE_RX_AUX,
};

enum
{
    RUNMODE_IDLE,
    RUNMODE_BINDING,
    RUNMODE_RUNNING,
    RUNMODE_ERROR,
}

typedef struct
{
    uint32_t magic_key;
    uint32_t tx_uid;
    uint32_t rx_uid;
    uint32_t hdr_chk;
    uint8_t  pkt_type;
}
pkthdr_t;

extern uint16_t channel[MULTIPROTOCOL_TOTAL_CHANNELS];
extern uint8_t rfm_buffer[64];
extern nvm_t nvm;
extern rfm22_modem_regs_t modem_params[RFM_AVAILABLE_PARAMS];

uint8_t get_hop_chan(uint8_t idx);

void build_packet(uint8_t* buffer, uint8_t pkt_type);

#ifdef __cplusplus
}
#endif

#endif
