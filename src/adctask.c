//#define SERIAL_TEST

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
#include "loudness.h"

#ifdef SERIAL_TEST
#include "serial_atmega328p.h"

char dbStr[32];
#endif

uint8_t avgLoudness = 0;

uint8_t getLoudness()
{
	return avgLoudness;
}

void ADCTask(PTASKPARM p)
{
	static int		i = 0;
	static int		on = 0;
	static uint16_t	loudnessSum = 0;
	uint8_t			loudness = 0;
	uint16_t *		pPeak = (uint16_t *)p;

	if (on) {
		turnOff(LED_ONBOARD);
		on = 0;
	}
	else {
		turnOn(LED_ONBOARD);
		on = 1;
	}

	loudness = pgm_read_word(&loudnessLookup[*pPeak - 1]);
	loudnessSum += loudness;

	i++;

	if (i == LOUDNESS_AVG_SAMPLE_SIZE) {
		i = 0;

		avgLoudness = (uint8_t)(loudnessSum >> LOUDNESS_AVG_BITSHIFT);

#ifdef SERIAL_TEST
		itoa((int)avgLoudness, dbStr, 10);
		dbStr[strlen(dbStr)] = '\n';

		txstr(dbStr, strlen(dbStr));
#endif

		loudnessSum = 0;
		*pPeak = 0;
	}
}
