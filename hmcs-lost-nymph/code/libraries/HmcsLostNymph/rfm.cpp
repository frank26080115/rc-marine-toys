#include "rfm.h"
#include <Arduino.h>
#include "bbspi.h"

rfm22_modem_regs_t modem_params[RFM_AVAILABLE_PARAMS] = {
    { 4800  , 0x1A, 0x40, 0x0A, 0xA1, 0x20, 0x4E, 0xA5, 0x00, 0x1B, 0x1E, 0x27, 0x52, 0x2C, 0x23, 0x30 }, // 50000 0x00
    { 9600  , 0x05, 0x40, 0x0A, 0xA1, 0x20, 0x4E, 0xA5, 0x00, 0x20, 0x24, 0x4E, 0xA5, 0x2C, 0x23, 0x30 }, // 25000 0x00
    { 19200 , 0x06, 0x40, 0x0A, 0xD0, 0x00, 0x9D, 0x49, 0x00, 0x7B, 0x28, 0x9D, 0x49, 0x2C, 0x23, 0x30 }, // 25000 0x01
    { 57600 , 0x05, 0x40, 0x0A, 0x45, 0x01, 0xD7, 0xDC, 0x03, 0xB8, 0x1E, 0x0E, 0xBF, 0x00, 0x23, 0x2E },
    { 125000, 0x8A, 0x40, 0x0A, 0x60, 0x01, 0x55, 0x55, 0x02, 0xAD, 0x1E, 0x20, 0x00, 0x00, 0x23, 0xC8 },
};

void rfmInit(uint8_t diversity)
{
  spiWriteRegister(RFM22B_INTEN2, 0x00);    // disable interrupts
  spiWriteRegister(RFM22B_INTEN1, 0x00);    // disable interrupts
  spiWriteRegister(RFM22B_XTALCAP, 0x7F);   // XTAL cap = 12.5pF
  spiWriteRegister(RFM22B_MCUCLK, 0x05);    // 2MHz clock

  spiWriteRegister(RFM22B_GPIOCFG2, (diversity ? 0x17 : 0xFD) ); // gpio 2 ant. sw, 1 if diversity on else VDD
  spiWriteRegister(RFM22B_PREAMLEN, (diversity ? 0x14 : 0x0A) );    // 40 bit preamble, 80 with diversity
  spiWriteRegister(RFM22B_IOPRTCFG, 0x00);    // gpio 0,1,2 NO OTHER FUNCTION.

  #ifdef SWAP_GPIOS
  spiWriteRegister(RFM22B_GPIOCFG0, 0x15);    // gpio0 RX State
  spiWriteRegister(RFM22B_GPIOCFG1, 0x12);    // gpio1 TX State
  #else
  spiWriteRegister(RFM22B_GPIOCFG0, 0x12);    // gpio0 TX State
  spiWriteRegister(RFM22B_GPIOCFG1, 0x15);    // gpio1 RX State
  #endif

  // Packet settings
  spiWriteRegister(RFM22B_DACTL, 0x8C);    // enable packet handler, msb first, enable crc,
  spiWriteRegister(RFM22B_HDRCTL1, 0x0F);    // no broadcast, check header bytes 3,2,1,0
  spiWriteRegister(RFM22B_HDRCTL2, 0x42);    // 4 byte header, 2 byte sync, variable packet size
  spiWriteRegister(RFM22B_PREATH, 0x2A);    // preamble detect = 5 (20bits), rssioff = 2
  spiWriteRegister(RFM22B_SYNC3, 0x2D);    // sync word 3
  spiWriteRegister(RFM22B_SYNC2, 0xD4);    // sync word 2
  spiWriteRegister(RFM22B_SYNC1, 0x00);    // sync word 1 (not used)
  spiWriteRegister(RFM22B_SYNC0, 0x00);    // sync word 0 (not used)
  spiWriteRegister(RFM22B_HDREN3, 0xFF);    // must set all bits
  spiWriteRegister(RFM22B_HDREN2, 0xFF);    // must set all bits
  spiWriteRegister(RFM22B_HDREN1, 0xFF);    // must set all bits
  spiWriteRegister(RFM22B_HDREN0, 0xFF);    // must set all bits

  spiWriteRegister(RFM22B_FREQOFF1, 0x00);    // no offset
  spiWriteRegister(RFM22B_FREQOFF2, 0x00);    // no offset
  spiWriteRegister(RFM22B_FHCH,        0x00);   // set to hop channel 0
}

void rfmClearFIFO(uint8_t diversity)
{
  //clear FIFO, disable multi-packet, enable diversity if needed
  //requires two write ops, set & clear
  spiWriteRegister(RFM22B_OPMODE2, (diversity ? 0x83 : 0x03) );
  spiWriteRegister(RFM22B_OPMODE2, (diversity ? 0x80 : 0x00) );
}

void rfmClearInterrupts(void)
{
  spiWriteRegister(RFM22B_INTEN1, 0x00);
  spiWriteRegister(RFM22B_INTEN2, 0x00);
}

void rfmClearIntStatus(void)
{
  spiReadRegister(RFM22B_INTSTAT1);
  spiReadRegister(RFM22B_INTSTAT2);
}

void rfmSendPacket(uint8_t* pkt, uint8_t size)
{
  spiWriteRegister(RFM22B_PKTLEN, size);   // total tx size
  for (uint8_t i = 0; i < size; i++) {
    spiWriteRegister(RFM22B_FIFO, pkt[i]);
  }
  spiWriteRegister(RFM22B_INTEN1, RFM22B_PACKET_SENT_INTERRUPT);
}

uint16_t rfmGetAFCC(void)
{
  return (((uint16_t) spiReadRegister(RFM22B_AFC0) << 2) | ((uint16_t) spiReadRegister(RFM22B_AFC1) >> 6));
}

uint8_t rfmGetGPIO1(void)
{
  return spiReadRegister(RFM22B_GPIOCFG1);
}

uint8_t rfmGetRSSI(void)
{
  return spiReadRegister(RFM22B_RSSI);
}

uint8_t rfmGetPacketLength(void)
{
  return spiReadRegister(RFM22B_RXPLEN);
}

void rfmGetPacket(uint8_t *buf, uint8_t size)
{
  // Send the package read command
  spiSendAddress(RFM22B_FIFO);
  for (uint8_t i = 0; i < size; i++) {
    buf[i] = spiReadData();
  }
  nSEL_HI();
}

int8_t rfmGetWholePacket(uint8_t *buf, uint8_t maxsize)
{
  int8_t gotlen = rfmGetPacketLength();
  // Send the package read command
  spiSendAddress(RFM22B_FIFO);
  for (uint8_t i = 0; i < maxsize || i < gotlen; i++) {
    uint8_t x = spiReadData();
    if (i < maxsize) {
        buf[i] = x;
    }
  }
  nSEL_HI();
  return gotlen;
}

void rfmSetTX(void)
{
  spiWriteRegister(RFM22B_OPMODE1, (RFM22B_OPMODE_TX | RFM22B_OPMODE_READY));
  delayMicroseconds(200); // allow for PLL & PA ramp-up, ~200us
}

void rfmSetRX(void)
{
  spiWriteRegister(RFM22B_INTEN1, RFM22B_RX_PACKET_RECEIVED_INTERRUPT);
  spiWriteRegister(RFM22B_OPMODE1, (RFM22B_OPMODE_RX | RFM22B_OPMODE_READY));
  delayMicroseconds(200);  // allow for PLL ramp-up, ~200us
}

void rfmSetCarrierFrequency(uint32_t f)
{
  uint16_t fb, fc, hbsel;
  if (f < 480000000) {
    hbsel = 0;
    fb = f / 10000000 - 24;
    fc = (f - (fb + 24) * 10000000) * 4 / 625;
  } else {
    hbsel = 1;
    fb = f / 20000000 - 24;
    fc = (f - (fb + 24) * 20000000) * 2 / 625;
  }
  spiWriteRegister(RFM22B_BANDSEL, 0x40 + (hbsel ? 0x20 : 0) + (fb & 0x1f));
  spiWriteRegister(RFM22B_CARRFREQ1, (fc >> 8));
  spiWriteRegister(RFM22B_CARRFREQ0, (fc & 0xff));
  delayMicroseconds(200); // VCO / PLL calibration delay
}

void rfmSetChannel(uint8_t ch)
{
  spiWriteRegister(RFM22B_FHCH, ch);
}

void rfmSetDirectOut(uint8_t enable)
{
 static uint8_t r1 = 0, r2 = 0, r3 = 0;
  if (enable) {
    r1 = spiReadRegister(RFM22B_DACTL);
    r2 = spiReadRegister(RFM22B_MODCTL2);
    r3 = spiReadRegister(RFM22B_FREQDEV);
    // setup for direct output, i.e. beacon tones
    spiWriteRegister(RFM22B_DACTL,   0x00);    //disable packet handling
    spiWriteRegister(RFM22B_MODCTL2, 0x12);    // trclk=[00] no clock, dtmod=[01] direct using SPI, fd8=0 eninv=0 modtyp=[10] FSK
    spiWriteRegister(RFM22B_FREQDEV, 0x02);    // fd (frequency deviation) 2*625Hz == 1.25kHz
  } else {
    // restore previous values
    spiWriteRegister(RFM22B_DACTL, r1);
    spiWriteRegister(RFM22B_MODCTL2, r2);
    spiWriteRegister(RFM22B_FREQDEV, r3); 
  }
}

void rfmSetHeader(uint8_t iHdr, uint8_t bHdr)
{
  spiWriteRegister(RFM22B_TXHDR3+iHdr, bHdr);
  spiWriteRegister(RFM22B_CHKHDR3+iHdr, bHdr);
}

void rfmSetModemRegs(rfm22_modem_regs_t* r)
{
  spiWriteRegister(RFM22B_IFBW,      r->r_1C);
  spiWriteRegister(RFM22B_AFCLPGR,   r->r_1D);
  spiWriteRegister(RFM22B_AFCTIMG,   r->r_1E);
  spiWriteRegister(RFM22B_RXOSR,     r->r_20);
  spiWriteRegister(RFM22B_NCOFF2,    r->r_21);
  spiWriteRegister(RFM22B_NCOFF1,    r->r_22);
  spiWriteRegister(RFM22B_NCOFF0,    r->r_23);
  spiWriteRegister(RFM22B_CRGAIN1,   r->r_24);
  spiWriteRegister(RFM22B_CRGAIN0,   r->r_25);
  spiWriteRegister(RFM22B_AFCLIM,    r->r_2A);
  spiWriteRegister(RFM22B_TXDR1,     r->r_6E);
  spiWriteRegister(RFM22B_TXDR0,     r->r_6F);
  spiWriteRegister(RFM22B_MODCTL1,   r->r_70);
  spiWriteRegister(RFM22B_MODCTL2,   r->r_71);
  spiWriteRegister(RFM22B_FREQDEV,   r->r_72);
}

void rfmSetPower(uint8_t power)
{
  spiWriteRegister(RFM22B_TXPOWER, power);
  delayMicroseconds(25); // PA ramp up/down time
}

void rfmSetReadyMode(void)
{
  spiWriteRegister(RFM22B_OPMODE1, RFM22B_OPMODE_READY);
}

void rfmSetStepSize(uint8_t sp)
{
  spiWriteRegister(RFM22B_FHS, sp);
}
