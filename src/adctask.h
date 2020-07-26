#include "adc_atmega328p.h"

#ifndef _INCL_ADCTASK
#define _INCL_ADCTASK

#define LOUDNESS_AVG_SAMPLE_SIZE		16
#define LOUDNESS_AVG_BITSHIFT			4

#define getADCBitSize()					((uint8_t)ADC_BIT_SIZE)

uint8_t     getLoudness();
void		ADCTask(PTASKPARM p);

#endif
