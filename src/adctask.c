#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <scheduler.h>

#include "led_utils.h"
#include "adctask.h"
#include "taskdef.h"
#include "adc_atmega328p.h"
#include "serial_atmega328p.h"
#include "rmslookup.h"
#include "dbalookup.h"

float db = 0.0;
char dbStr[32];

void getDB(uint8_t * integerPart, uint8_t * fractionalPart)
{
	*integerPart = (uint8_t)db;
	*fractionalPart = (uint8_t)((db - (float)(*integerPart)) * 100);
}

void ADCTask(PTASKPARM p)
{
	static int		i = 0;
	static int		on = 0;
	static uint32_t	rmsSum = 0;
	uint32_t		avgRMS;
	uint16_t *		pPeak = (uint16_t *)p;

	if (on) {
		turnOff(LED_ONBOARD);
		on = 0;
	}
	else {
		turnOn(LED_ONBOARD);
		on = 1;
	}

	rmsSum += pgm_read_word(&rmsLookup[*pPeak]);

	i++;

	if (i == RMS_AVG_SAMPLE_SIZE) {
		i = 0;

		avgRMS = rmsSum >> 6;

		db = pgm_read_float(&dbaLookup[avgRMS]);

		itoa((int)db, dbStr, 10);
		dbStr[strlen(dbStr)] = '\n';

		txstr(dbStr, strlen(dbStr));

		rmsSum = 0;
		*pPeak = 0;
	}
}
