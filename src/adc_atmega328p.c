#include <stddef.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

#include "rtc_atmega328p.h"
#include "adctask.h"
#include "taskdef.h"
#include "adc_atmega328p.h"
#include "rmslookup.h"

#define ADC_ZERO_POINT					(MAX_ADC_VALUE >> 1)

static volatile uint16_t peakValue = 0;
static uint16_t rmsWindowSize = ADC_DEFAULT_WINDOW_SIZE;

void setWindowSize(uint16_t windowSize)
{
	rmsWindowSize = windowSize;
}

uint16_t getWindowSize()
{
	return rmsWindowSize;
}

void setupADC(void)
{
	ADMUX	= _BV(REFS0) | ADC_CHANNEL0;
	ADCSRA	= _BV(ADEN) | _BV(ADIE) | ADC_PRESCALER_DIV64;
}

/*
** ADC Conversion complete interrupt handler
*/
ISR(ADC_vect, ISR_BLOCK)
{
	static int 			i = 0;
	uint16_t register	value;
	
	/*
	** 10-bit result from ADC
	** Read LSB first then MSB
	*/
	value = ADCL;
	value |= ((ADCH & 0x03) << 8);

	/*
	** Rectify our sample and zero reference, e.g. samples below the 'zero' point are
	** given a value at the same level above the zero point. All values will be between
	** 0 and 511...
	*/
	value = (value < ADC_ZERO_POINT) ? (ADC_ZERO_POINT - value) : (value - ADC_ZERO_POINT);

	if (value > peakValue) {
		peakValue = value;
	}

	i++;
	
	triggerADC();

	if (i == rmsWindowSize) {
		scheduleTaskOnce(TASK_ADC, RUN_NOW, (PTASKPARM)&peakValue);

		i = 0;
		peakValue = 0;
	}
}
