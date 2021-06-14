#ifndef _AADC_H_
#define _AADC_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void AADC_task(void);
bool AADC_hasNew(uint8_t c);
void AADC_clearNew(uint8_t c);
double AADC_read(uint8_t c);
uint16_t AADC_read16(uint8_t c);

#ifdef __cplusplus
}
#endif

#endif
