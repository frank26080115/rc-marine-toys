#ifndef _RFM_H_
#define _RFM_H_

#include "rfm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

void rfmInit(uint8_t diversity);
void rfmClearInterrupts(void);
void rfmClearIntStatus(void);
void rfmClearFIFO(uint8_t diversity);
void rfmSendPacket(uint8_t* pkt, uint8_t size);

uint16_t rfmGetAFCC(void);
uint8_t rfmGetGPIO1(void);
uint8_t rfmGetRSSI(void);
uint8_t rfmGetPacketLength(void);
void rfmGetPacket(uint8_t *buf, uint8_t size);

void rfmSetTX(void);
void rfmSetRX(void);
void rfmSetCarrierFrequency(uint32_t f);
void rfmSetChannel(uint8_t ch);
void rfmSetDirectOut(uint8_t enable);
void rfmSetHeader(uint8_t iHdr, uint8_t bHdr);
void rfmSetModemRegs(rfm22_modem_regs_t* r);
void rfmSetPower(uint8_t p);
void rfmSetReadyMode(void);
void rfmSetStepSize(uint8_t sp);

#ifdef __cplusplus
}
#endif

#endif
