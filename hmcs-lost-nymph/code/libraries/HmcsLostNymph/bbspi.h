#ifndef _BBSPI_H_
#define _BBSPI_H_

#include <Arduino.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __AVR__

#define _NOP() __asm__ __volatile__("nop")

#define  nSEL_OUT() DDRD  |=  _BV(4)
#define  nSEL_HI()  PORTD |=  _BV(4)
#define  nSEL_LO()  PORTD &= ~_BV(4)

#define  SCK_OUT()  DDRD  |=  _BV(7)
#define  SCK_HI()   PORTD |=  _BV(7)
#define  SCK_LO()   PORTD &= ~_BV(7)

#define  SDI_OUT()  DDRB  |=  _BV(0)
#define  SDI_HI()   PORTB |=  _BV(0)
#define  SDI_LO()   PORTB &= ~_BV(0)

#define  SDO_IN()   DDRD   &= ~_BV(1)
#define  SDO_LO()   PORTD  &= ~_BV(1)
#define  SDO_1()    ((PINB &   _BV(1)) != 0x00)
#define  SDO_0()    ((PINB &   _BV(1)) == 0x00)

#else

#define _NOP() __nop()

#define  nSEL_OUT() pinMode(x, OUTPUT)
#define  nSEL_HI()  digitalWriteFast(x, HIGH)
#define  nSEL_LO()  digitalWriteFast(x, LOW)

#ifdef FORCE_SPI_BITBANG

#define  SCK_OUT()  pinMode(x, OUTPUT)
#define  SCK_HI()   digitalWriteFast(x, HIGH)
#define  SCK_LO()   digitalWriteFast(x, LOW)

#define  SDI_OUT()  pinMode(x, OUTPUT)
#define  SDI_HI()   digitalWriteFast(x, HIGH)
#define  SDI_LO()   digitalWriteFast(x, LOW)

#define  SDO_IN()   pinMode(x, INPUT)
#define  SDO_LO()   _NOP()
#define  SDO_1()    (digitalReadFast(x) != LOW)
#define  SDO_0()    (digitalReadFast(x) == LOW)

#endif

#endif

uint8_t spiReadBit(void);
uint8_t spiReadData(void);
uint8_t spiReadRegister(uint8_t address);

void spiWriteBit(uint8_t b);
void spiWriteData(uint8_t i);
void spiWriteRegister(uint8_t address, uint8_t data);

void spiSendAddress(uint8_t i);
void spiSendCommand(uint8_t command);

void spi_hwInit(void);

#ifdef __cplusplus
}
#endif

#endif
