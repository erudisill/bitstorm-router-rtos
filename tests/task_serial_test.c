/*
 * task_serial_test.c
 *
 *  Created on: Feb 10, 2015
 *      Author: ericrudisill
 */

#include <stdio.h>
#include "task_serial_test.h"

#define BUFFER_MAX		64

static signed char inBuffer[BUFFER_MAX + 1];
static int bufferIndex;

static void serial_putsz(xComPortHandle hnd, char * data) {
	for (int i=0; data[i]; xSerialPutChar(hnd, data[i++], 0)) ;
}

static void serial_flush(xComPortHandle hnd) {
	inBuffer[bufferIndex] = 0;
	serial_putsz(hnd, (char *)inBuffer);
	bufferIndex = 0;
}

/*
static portTASK_FUNCTION(task_cli, params) {
	portBASE_TYPE result;
	UBaseType_t hwm;
	signed char inChar;
	xComPortHandle pxHandle;

	pxHandle = xSerialPortInitMinimal(0, 38400, 64);

	serial_putsz(pxHandle, "\r\n[ALIVE]\r\n\r\n> ");

	bufferIndex = 0;

	for (;;) {

		result = xSerialGetChar(pxHandle, &inChar, 0xFFFF);

		if (result == pdTRUE) {
			if (inChar == '\r') {
				hwm = uxTaskGetStackHighWaterMark(NULL);
				serial_putsz(pxHandle, "\r\nECHO: ");
				serial_flush(pxHandle);
				hwm = uxTaskGetStackHighWaterMark(NULL);
				sprintf((char*)inBuffer, "\r\n%d >", hwm);
				serial_putsz(pxHandle, (char*)inBuffer);
			} else {
				inBuffer[bufferIndex++] = inChar;
				xSerialPutChar(pxHandle, inChar, 0);
				if (bufferIndex >= BUFFER_MAX) {
					serial_putsz(pxHandle, "\r\nOVERFLOW: ");
					serial_flush(pxHandle);
					hwm = uxTaskGetStackHighWaterMark(NULL);
					sprintf((char*)inBuffer, "\r\n%d >", hwm);
					serial_putsz(pxHandle, (char*)inBuffer);
				}
			}
		}
	}
}
*/

static portTASK_FUNCTION(task_ble, params) {
	portBASE_TYPE result;
	signed char inChar;
	xComPortHandle pxIn;
	xComPortHandle pxOut;

	pxOut = xSerialPortInitMinimal(0, 38400, 64);
	pxIn = xSerialPortInitMinimal(1, 38400, 64);

	serial_putsz(pxOut, "\r\n[ALIVE]\r\n\r\n");

	bufferIndex = 0;

	DDRD |= _BV(PD5);		// CTS - output
	DDRD &= ~_BV(PD4);		// RTS - input

	PORTD &= ~_BV(PD5);		// lower CTS to start the BLE stream


	for (;;) {

		result = xSerialGetChar(pxIn, &inChar, 0xFFFF);

		if (result == pdTRUE) {
			if (inChar == '\n') {
				serial_putsz(pxOut, "\r\n[RCV] ");
				serial_flush(pxOut);
			} else {
				inBuffer[bufferIndex++] = inChar;
				if (bufferIndex >= BUFFER_MAX) {
					serial_putsz(pxOut, "\r\n[OVF] ");
					serial_flush(pxOut);
				}
			}
		}
	}
}

void task_serial_test_start( UBaseType_t uxPriority )
{
//	xTaskCreate( task_cli, "cli", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
	xTaskCreate( task_ble, "ble", configMINIMAL_STACK_SIZE, NULL, uxPriority, ( TaskHandle_t * ) NULL );
}
