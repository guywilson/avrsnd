#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <scheduler.h>

#include "adctask.h"
#include "taskdef.h"
#include "adc_atmega328p.h"
#include "rmslookup.h"

void ADCTask(PTASKPARM p)
{
	static int		i = 0;
	static uint32_t	rmsSum = 0;
	uint32_t		avgRMS;
	uint16_t 		peak = *(uint16_t *)p;

	rmsSum += rmsLookup[peak];

	i++;

	if (i == RMS_AVG_SAMPLE_SIZE) {
		avgRMS = rmsSum >> 7;
	}
}
