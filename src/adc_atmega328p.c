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

static uint16_t rms;

void setupADC(void)
{
	ADMUX	= _BV(REFS0) | ADC_CHANNEL0;
	ADCSRA	= _BV(ADEN) | _BV(ADIE) | ADC_PRESCALER_DIV128;
}

/*
** ADC Conversion complete interrupt handler
*/
void handleADConversionComplete()
{
	static uint16_t		peak = 0;
	static int			i = 0;
	uint16_t			value;
	
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

	if (value > peak) {
		peak = value;
	}

	i++;

	if (i == ADC_WINDOW_SIZE) {
		rms = rmsLookup[peak];

		i = 0;
		peak = 0;

		scheduleTaskOnce(TASK_ADC, RUN_NOW, (PTASKPARM)&rms);
	}
}
