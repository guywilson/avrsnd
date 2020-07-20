#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <scheduler.h>

#include "adctask.h"
#include "taskdef.h"
#include "adc_atmega328p.h"
#include "rmslookup.h"
#include "dbalookup.h"

float db = 0.0;

void getDB(uint8_t * integerPart, uint8_t * fractionalPart)
{
	*integerPart = (uint8_t)db;
	*fractionalPart = (uint8_t)((db - (float)(*integerPart)) * 100);
}

void ADCTask(PTASKPARM p)
{
	static int		i = 0;
	static uint32_t	rmsSum = 0;
	uint32_t		avgRMS;
	uint16_t 		peak = *(uint16_t *)p;

	rmsSum += pgm_read_word(rmsLookup[peak]);

	i++;

	if (i == RMS_AVG_SAMPLE_SIZE) {
		avgRMS = rmsSum >> 7;

		db = pgm_read_float(dbaLookup[avgRMS]);
	}
}
