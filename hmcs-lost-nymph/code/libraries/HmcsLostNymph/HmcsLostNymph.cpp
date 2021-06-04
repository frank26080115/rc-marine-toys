
uint16_t channel[MULTIPROTOCOL_TOTAL_CHANNELS];
uint8_t rfm_buffer[64];

uint8_t get_hop_chan(uint8_t idx)
{
    uint32_t xored_id[2];
    uint8_t* p;
    xored_id[0] = nvm.tx_uid;
    xored_id[1] = nvm.rx_uid;
    p = (uint8_t*)(xored_id);
    return p[idx % 8];
}

void build_packet(uint32_t tx_uid, uint32_t rx_uid, uint8_t* buffer, uint8_t pkt_type)
{
    pkthdr_t* ptr = (pkthdr_t*)buffer;
    ptr->magic_key = MAGIC_KEY;
    ptr->tx_uid = tx_uid;
    ptr->rx_uid = rx_uid;
    ptr->hdr_chk = MAGIC_KEY ^ tx_uid ^ rx_uid ^ PRIVATE_KEY;
    ptr->pkt_type = pkt_type;
}

bool check_packet(uint8_t* buffer, uint32_t tx_uid, uint32_t rx_uid)
{
    pkthdr_t* ptr = (pkthdr_t*)buffer;
    if (ptr->magic_key != MAGIC_KEY)
    {
        return false;
    }
    if (tx_uid != 0 && ptr->tx_uid != tx_uid)
    {
        return false;
    }
    if (rx_uid != 0 && ptr->rx_uid != rx_uid)
    {
        return false;
    }
    if (ptr->hdr_chk != (MAGIC_KEY ^ ptr->tx_uid ^ ptr->rx_uid ^ PRIVATE_KEY))
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
    for (uint8_t i = 0; i < MULTIPROTOCOL_TOTAL_CHANNELS; i++)
    {
        if (channel[i] > (PULSE_CENTER_US + RC_MINIMUM_MOVE) || channel[i] < (PULSE_CENTER_US - RC_MINIMUM_MOVE))
        {
            return true;
        }
    }
    return false;
}