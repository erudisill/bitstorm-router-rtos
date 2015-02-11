
#include <stdlib.h>
#include <string.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Application include files. */
#include "task_blinky.h"
#include "task_serial_test.h"

/*-----------------------------------------------------------*/

int main(void) {

	task_blinky_start( (tskIDLE_PRIORITY+1) );
	task_serial_test_start( (tskIDLE_PRIORITY+2) );

	vTaskStartScheduler();

	return 0;
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ) {
	led_alert_on();
}
/*-----------------------------------------------------------*/

