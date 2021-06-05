#ifndef _RFM_DEFS_H_
#define _RFM_DEFS_H_

#define RFM_PKT_LEN         64

#define RFM22B_INTSTAT1     0x03
#define RFM22B_INTSTAT2     0x04
#define RFM22B_INTEN1       0x05
#define RFM22B_INTEN2       0x06
#define RFM22B_OPMODE1      0x07
#define RFM22B_OPMODE2      0x08
#define RFM22B_XTALCAP      0x09
#define RFM22B_MCUCLK       0x0A
#define RFM22B_GPIOCFG0     0x0B
#define RFM22B_GPIOCFG1     0x0C
#define RFM22B_GPIOCFG2     0x0D
#define RFM22B_IOPRTCFG     0x0E

#define RFM22B_IFBW         0x1C
#define RFM22B_AFCLPGR      0x1D
#define RFM22B_AFCTIMG      0x1E
#define RFM22B_RXOSR        0x20
#define RFM22B_NCOFF2       0x21
#define RFM22B_NCOFF1       0x22
#define RFM22B_NCOFF0       0x23
#define RFM22B_CRGAIN1      0x24
#define RFM22B_CRGAIN0      0x25
#define RFM22B_RSSI         0x26
#define RFM22B_AFCLIM       0x2A
#define RFM22B_AFC0         0x2B
#define RFM22B_AFC1         0x2C

#define RFM22B_DACTL        0x30
#define RFM22B_HDRCTL1      0x32
#define RFM22B_HDRCTL2      0x33
#define RFM22B_PREAMLEN     0x34
#define RFM22B_PREATH       0x35
#define RFM22B_SYNC3        0x36
#define RFM22B_SYNC2        0x37
#define RFM22B_SYNC1        0x38
#define RFM22B_SYNC0        0x39

#define RFM22B_TXHDR3       0x3A
#define RFM22B_TXHDR2       0x3B
#define RFM22B_TXHDR1       0x3C
#define RFM22B_TXHDR0       0x3D
#define RFM22B_PKTLEN       0x3E
#define RFM22B_CHKHDR3      0x3F
#define RFM22B_CHKHDR2      0x40
#define RFM22B_CHKHDR1      0x41
#define RFM22B_CHKHDR0      0x42
#define RFM22B_HDREN3       0x43
#define RFM22B_HDREN2       0x44
#define RFM22B_HDREN1       0x45
#define RFM22B_HDREN0       0x46
#define RFM22B_RXPLEN       0x4B

#define RFM22B_TXPOWER      0x6D
#define RFM22B_TXDR1        0x6E
#define RFM22B_TXDR0        0x6F

#define RFM22B_MODCTL1      0x70
#define RFM22B_MODCTL2      0x71
#define RFM22B_FREQDEV      0x72
#define RFM22B_FREQOFF1     0x73
#define RFM22B_FREQOFF2     0x74
#define RFM22B_BANDSEL      0x75
#define RFM22B_CARRFREQ1    0x76
#define RFM22B_CARRFREQ0    0x77
#define RFM22B_FHCH         0x79
#define RFM22B_FHS          0x7A
#define RFM22B_TX_FIFO_CTL1 0x7C
#define RFM22B_TX_FIFO_CTL2 0x7D
#define RFM22B_RX_FIFO_CTL  0x7E
#define RFM22B_FIFO         0x7F

#define RFM22B_OPMODE_POWERDOWN    0x00
#define RFM22B_OPMODE_READY        0x01  // enable READY mode
#define RFM22B_OPMODE_TUNE         0x02  // enable TUNE mode
#define RFM22B_OPMODE_RX           0x04  // enable RX mode
#define RFM22B_OPMODE_TX           0x08  // enable TX mode
#define RFM22B_OPMODE_32K          0x10  // enable internal 32k xtal
#define RFM22B_OPMODE_WUT          0x40  // wake up timer
#define RFM22B_OPMODE_LBD          0x80  // low battery detector

#define RFM22B_PACKET_SENT_INTERRUPT          0x04
#define RFM22B_RX_PACKET_RECEIVED_INTERRUPT   0x02

typedef struct rfm22_modem_regs {
  uint32_t bps;
  uint8_t  r_1C, r_1D, r_1E, r_20, r_21, r_22, r_23, r_24, r_25, r_2A, r_6E, r_6F, r_70, r_71, r_72;
}
rfm22_modem_regs_t;

#define RFM_AVAILABLE_PARAMS  5
#define RFM_DEFAULT_PARAM_IDX 2
rfm22_modem_regs_t modem_params[RFM_AVAILABLE_PARAMS] = {
    { 4800  , 0x1A, 0x40, 0x0A, 0xA1, 0x20, 0x4E, 0xA5, 0x00, 0x1B, 0x1E, 0x27, 0x52, 0x2C, 0x23, 0x30 }, // 50000 0x00
    { 9600  , 0x05, 0x40, 0x0A, 0xA1, 0x20, 0x4E, 0xA5, 0x00, 0x20, 0x24, 0x4E, 0xA5, 0x2C, 0x23, 0x30 }, // 25000 0x00
    { 19200 , 0x06, 0x40, 0x0A, 0xD0, 0x00, 0x9D, 0x49, 0x00, 0x7B, 0x28, 0x9D, 0x49, 0x2C, 0x23, 0x30 }, // 25000 0x01
    { 57600 , 0x05, 0x40, 0x0A, 0x45, 0x01, 0xD7, 0xDC, 0x03, 0xB8, 0x1E, 0x0E, 0xBF, 0x00, 0x23, 0x2E },
    { 125000, 0x8A, 0x40, 0x0A, 0x60, 0x01, 0x55, 0x55, 0x02, 0xAD, 0x1E, 0x20, 0x00, 0x00, 0x23, 0xC8 },
};

#endif