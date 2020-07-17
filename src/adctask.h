#include "adc_atmega328p.h"

#ifndef _INCL_ADCTASK
#define _INCL_ADCTASK

#define RMS_AVG_SAMPLE_SIZE				64
#define RMS_AVG_BITSHIFT				 7

#define getADCBitSize()					((uint8_t)ADC_BIT_SIZE)

void		ADCTask(PTASKPARM p);

#endif
