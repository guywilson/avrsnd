#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <scheduler.h>
#include <schederr.h>

#include "taskdef.h"
#include "heartbeat.h"

#include "led_utils.h"
#include "wdt_atmega328p.h"
#include "rtc_atmega328p.h"
#include "adc_atmega328p.h"
#include "twi_atmega328p.h"
#include "adctask.h"
#include "wdttask.h"
#include "version.h"

void setup(void)
{
	/*
	 * Disable interrupts...
	 */
	cli();

	setupLEDPin();
	setupRTC();
	setupWDT();
	setupTWI();
	setupADC();

	/*
	 * Enable interrupts...
	 */
    sei();
}

int main(void)
{
	setup();

	/*
	 * Switch on the LED so we know when the board has reset...
	 */
	turnOn(LED_ONBOARD);

	initScheduler(NUM_TASKS);

	registerTask(TASK_WDT, &wdtTask);
	registerTask(TASK_HEARTBEAT, &HeartbeatTask);
	registerTask(TASK_ADC, &ADCTask);

	scheduleTask(
			TASK_WDT, 
			rtc_val_ms(125), 
			NULL);

	scheduleTaskOnce(
			TASK_HEARTBEAT,
			rtc_val_ms(50),
			NULL);

    /*
     * Trigger the first ADC conversion...
     */
	triggerADC();

	/*
	** Start the scheduler...
	*/
	schedule();

	/*
	 * It should never get here...
	 */
	return -1;
}
