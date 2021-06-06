#include "bbspi.h"
#include <Arduino.h>
#include <SPI.h>

#if defined(__AVR__) || defined(FORCE_SPI_BITBANG)

uint8_t spiReadBit(void)
{
  uint8_t r = 0;
  SCK_HI();
  _NOP();
  if (SDO_1()) {
    r = 1;
  }
  SCK_LO();
  _NOP();
  return r;
}

uint8_t spiReadData(void)
{
  uint8_t Result = 0;
  SCK_LO();
  for (uint8_t i = 0; i < 8; i++) {   //read fifo data byte
    Result = (Result << 1) + spiReadBit();
  }
  return(Result);
}

void spiWriteBit(uint8_t b)
{
  if (b) {
    SCK_LO();
    _NOP();
    SDI_HI();
    _NOP();
    SCK_HI();
    _NOP();
  } else {
    SCK_LO();
    _NOP();
    SDI_LO();
    _NOP();
    SCK_HI();
    _NOP();
  }
}

void spiWriteData(uint8_t i)
{
  for (uint8_t n = 0; n < 8; n++) {
    spiWriteBit(i & 0x80);
    i = i << 1;
  }
  SCK_LO();
}

void spiSendCommand(uint8_t command)
{
  nSEL_HI();
  SCK_LO();
  nSEL_LO();
  for (uint8_t n = 0; n < 8 ; n++) {
    spiWriteBit(command & 0x80);
    command = command << 1;
  }
  SCK_LO();
}

void spi_hwInit()
{
    nSEL_OUT();
    nSEL_HI();
    SCK_OUT();
    SCK_LO();
    SDI_OUT();
    SDO_IN();
    SDO_LO();
}

#else // ARM and using HW SPI

void spi_hwInit()
{
    nSEL_OUT();
    nSEL_HI();
    SPI.begin();
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
}

void spiWriteData(uint8_t i)
{
  SPI.transfer(i);
}

void spiSendCommand(uint8_t command)
{
  nSEL_HI();
  delayMicroseconds(1);
  nSEL_LO();
  SPI.transfer(command);
}

uint8_t spiReadData(void)
{
  uint8_t x = 0;
  x = SPI.transfer(0);
  return x;
}

#endif

uint8_t spiReadRegister(uint8_t address)
{
  uint8_t x;
  spiSendAddress(address);
  x = spiReadData();
  nSEL_HI();
  return x;
}

void spiSendAddress(uint8_t i)
{
  spiSendCommand(i & 0x7f);
}

void spiWriteRegister(uint8_t address, uint8_t data)
{
  address |= 0x80;
  spiSendCommand(address);
  spiWriteData(data);
  nSEL_HI();
}
