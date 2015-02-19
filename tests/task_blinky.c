/*
 * task_blinky.c
 *
 *  Created on: Feb 10, 2015
 *      Author: ericrudisill
 */

#include <stddef.h>
#include "task_blinky.h"

static portTASK_FUNCTION(task_blinky, pvParameters) {

	portTickType xLastWakeTime;
	const portTickType xFrequency = 1000;
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		led_toggle();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

void task_blinky_start( UBaseType_t uxPriority )
{
	led_init();

	xTaskCreate( task_blinky, "blinky", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
}
