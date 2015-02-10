/*
 * task_serial_test.c
 *
 *  Created on: Feb 10, 2015
 *      Author: ericrudisill
 */

#include "task_serial_test.h"

#define BUFFER_MAX		20

static signed char inBuffer[BUFFER_MAX + 1];
static int bufferIndex;

static void serial0_putsz(char * data) {
	for (int i=0; data[i]; xSerialPutChar(NULL, data[i++], 0)) ;
}

static void serial0_flush(void) {
	inBuffer[bufferIndex] = 0;
	serial0_putsz((char *)inBuffer);
	bufferIndex = 0;
}

static portTASK_FUNCTION(task_tx, params) {
	portBASE_TYPE result;
	signed char inChar;

	xSerialPortInitMinimal(38400, 64);

	serial0_putsz("\r\n[ALIVE]\r\n\r\n> ");

	bufferIndex = 0;

	for (;;) {

		result = xSerialGetChar(NULL, &inChar, 100);

		if (result == pdTRUE) {
			if (inChar == '\r') {
				serial0_putsz("\r\nECHO: ");
				serial0_flush();
				serial0_putsz("\r\n> ");
			} else {
				inBuffer[bufferIndex++] = inChar;
				xSerialPutChar(NULL, inChar, 0);
				if (bufferIndex >= BUFFER_MAX) {
					serial0_putsz("\r\nOVERFLOW: ");
					serial0_flush();
					serial0_putsz("\r\n> ");
				}
			}
		}
	}
}

void task_serial_test_start( UBaseType_t uxPriority )
{
	xTaskCreate( task_tx, "serial0", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
}
