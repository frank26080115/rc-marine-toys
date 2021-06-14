#include "HmcsLostNymph.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "bbspi.h"

uint16_t channel[MULTIPROTOCOL_TOTAL_CHANNELS];
uint8_t  rfm_buffer[RFM_PKT_LEN];
#ifdef HLN_SEND_COMPRESSED
uint8_t  sbus_buffer[RFM_PKT_LEN];
#endif
nvm_t nvm;
uint32_t link_quality;

uint8_t get_hop_chan(uint8_t idx)
{
    #ifndef DISABLE_HOP
    uid_t xored_id[2];
    uint8_t* p;
    xored_id[0] = nvm.tx_uid;
    xored_id[1] = nvm.rx_uid;
    p = (uint8_t*)(xored_id);
    return p[idx % (sizeof(uid_t) * 2)];
    #else
    return RFM_BIND_CHANNEL;
    #endif
}

uid_t calc_hdrchk(uid_t tx_uid, uid_t rx_uid)
{
    uint32_t x = MAGIC_KEY;
    x ^= tx_uid;
    x ^= rx_uid;
    x ^= PRIVATE_KEY;
    return (uid_t)x;
}

void build_packet(uid_t tx_uid, uid_t rx_uid, uint8_t* buffer, uint8_t pkt_type)
{
    pkthdr_t* ptr = (pkthdr_t*)buffer;
    #ifdef EXPLICIT_MAGIC
    ptr->magic_key = MAGIC_KEY;
    #endif
    ptr->tx_uid = tx_uid;
    ptr->rx_uid = rx_uid;
    ptr->hdr_chk = calc_hdrchk(tx_uid, rx_uid);
    ptr->pkt_type = pkt_type;
}

bool check_packet(uint8_t* buffer, uid_t tx_uid, uid_t rx_uid)
{
    pkthdr_t* ptr = (pkthdr_t*)buffer;
    #ifdef EXPLICIT_MAGIC
    if (ptr->magic_key != MAGIC_KEY)
    {
        return false;
    }
    #endif
    if (tx_uid != 0 && ptr->tx_uid != tx_uid)
    {
        return false;
    }
    if (rx_uid != 0 && ptr->rx_uid != rx_uid)
    {
        return false;
    }
    if (ptr->hdr_chk != calc_hdrchk(ptr->tx_uid, ptr->rx_uid))
    {
        return false;
    }
    return true;
}

void center_all_channels()
{
    for (uint8_t i = 0; i < MULTIPROTOCOL_TOTAL_CHANNELS; i++)
    {
        channel[i] = PULSE_CENTER_US;
    }
}

bool channels_has_movement()
{
    for (uint8_t i = 0; i < RC_USED_CHANNELS; i++)
    {
        if (channel[i] > (PULSE_CENTER_US + RC_DEADZONE_US) || channel[i] < (PULSE_CENTER_US - RC_DEADZONE_US))
        {
            return true;
        }
    }
    return false;
}

uid_t gen_rand_uid()
{
    uint32_t bind_uid;
    srand(micros() ^ millis());
    do
    {
        bind_uid = (uint32_t)rand();
    }
    while (bind_uid == 0 || bind_uid == (uint32_t)0xFFFFFFFF);
    return (uid_t)bind_uid;
}

void nvm_save(uint8_t idx)
{
    uint8_t* ptr = (uint8_t*)&nvm;
    uint32_t addr = idx * sizeof(nvm_t);
    for (uint8_t i = 0; i < sizeof(nvm_t); i++, addr++)
    {
        ptr[i] = EEPROM.read(addr);
    }
}

void nvm_load(uint8_t idx)
{
    uint8_t* ptr = (uint8_t*)&nvm;
    uint32_t addr = idx * sizeof(nvm_t);
    for (uint8_t i = 0; i < sizeof(nvm_t); i++, addr++)
    {
        EEPROM.write(addr, ptr[i]);
    }
}

void radio_init()
{
    spi_hwInit();
    rfmSetReadyMode();      // turn on XTAL
    delayMicroseconds(600); // time to settle
    rfmClearIntStatus();
    rfmInit(0);
    rfmSetStepSize(RFM_FREQHOP_STEPSIZE);

    uint32_t magic = MAGIC_KEY;
    for (uint8_t i = 0; i < 4; i++)
    {
        rfmSetHeader(i, (magic >> 24) & 0xFF);
        magic = magic << 8; // advance to next byte
    }

    rfmSetModemRegs(&modem_params[RFM_DEFAULT_PARAM_IDX]);
    rfmSetPower(RFM_TX_POWER);
    rfmSetCarrierFrequency(RFM_NOMINAL_FREQ);
}

uint16_t multiproto_2_pulseUs(uint16_t x)
{
    int32_t value = x;
    // from https://github.com/pascallanger/DIY-Multiprotocol-TX-Module/blob/master/Multiprotocol/Multiprotocol.h
    value = (((value - 204) * 1000) / 1639) + 1000;
    return value;
}

void decode_sbus(uint8_t* packet, uint16_t* channels)
{
    ppm_msg_t* ptr = (ppm_msg_t*)(packet);
    uint8_t set;
    for (set = 0; set < 2; set++)
    {
        channel[(set << 3) + 0] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch0);
        channel[(set << 3) + 1] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch1);
        channel[(set << 3) + 2] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch2);
        channel[(set << 3) + 3] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch3);
        channel[(set << 3) + 4] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch4);
        channel[(set << 3) + 5] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch5);
        channel[(set << 3) + 6] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch6);
        channel[(set << 3) + 7] = multiproto_2_pulseUs(ptr->sbus.ch[set].ch7);
    }
}

#ifdef USE_LQI
void linkquality_inc()
{
    if (link_quality <= 0)
    {
        link_quality = 1;
    }
    else
    {
        link_quality <<= 1;
        link_quality |= 1;
    }
}

void linkquality_dec()
{
    link_quality >>= 1;
}

void linkquality_zero()
{
    link_quality = 0;
}

uint8_t linkquality_get()
{
    int i;
    for (i = 32; i >= 0; i--)
    {
        uint32_t j = (1 << i);
        if ((link_quality & j) != 0)
        {
            return i * 4;
        }
    }
}
#endif
