#include "adc_atmega328p.h"

#ifndef _INCL_ADCTASK
#define _INCL_ADCTASK

#define getADCBitSize()					((uint8_t)ADC_BIT_SIZE)

void		ADCTask(PTASKPARM p);

#endif
