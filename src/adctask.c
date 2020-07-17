#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <scheduler.h>

#include "adctask.h"
#include "taskdef.h"
#include "adc_atmega328p.h"

void ADCTask(PTASKPARM p)
{
	uint16_t rms = *(uint16_t *)p;

	/*
	** Trigger next conversion...
	*/
	triggerADC();
}
