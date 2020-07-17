#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <scheduler.h>
#include <schederr.h>

#include "taskdef.h"
#include "heartbeat.h"

#include "led_utils.h"
#include "rtc_atmega328p.h"
#include "adc_atmega328p.h"
#include "adctask.h"

void setup(void)
{
	/*
	 * Disable interrupts...
	 */
	cli();

	setupLEDPin();
	setupRTC();
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

	registerTask(TASK_HEARTBEAT, &HeartbeatTask);
	registerTask(TASK_ADC, &ADCTask);

	scheduleTask(
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
