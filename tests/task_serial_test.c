/*
 * task_serial_test.c
 *
 *  Created on: Feb 10, 2015
 *      Author: ericrudisill
 */

#include <stdio.h>
#include "task_serial_test.h"

#define BUFFER_MAX		64
#define BUFFER_SIZE     (BUFFER_MAX + 1)
#define QUEUE_SIZE		10
#define QUEUE_TICKS		10

static signed char inBuffer [BUFFER_SIZE];
static signed char outBuffer[BUFFER_SIZE];
static int bufferIndex;

QueueHandle_t xPacketQueue;

static portTASK_FUNCTION(task_ble, params) {
	BaseType_t result;
	signed char inChar;
	xComPortHandle pxIn;

	pxIn = xSerialPortInitMinimal(1, 38400, 64);

	bufferIndex = 0;

	DDRD |= _BV(PD5);		// CTS - output
	DDRD &= ~_BV(PD4);		// RTS - input

	PORTD &= ~_BV(PD5);		// lower CTS to start the BLE stream

	for (;;) {

		result = xSerialGetChar(pxIn, &inChar, 0xFFFF);

		if (result == pdTRUE) {
			if (inChar == '\n') {
				led_alert_off();
				inBuffer[bufferIndex] = 0;
				result = xQueueSendToBack( xPacketQueue, inBuffer, 0);
				bufferIndex = 0;
				if (result != pdTRUE) {
					led_alert_on();
				}
			} else {
				inBuffer[bufferIndex++] = inChar;
				if (bufferIndex >= BUFFER_MAX) {
					led_alert_on();
					bufferIndex = 0;
				}
			}
		}
	}
}

static void serial_putsz(xComPortHandle hnd, char * data) {
	for (int i=0; data[i]; xSerialPutChar(hnd, data[i++], 0)) ;
}

static portTASK_FUNCTION(task_wan, params) {
	BaseType_t result;
	xComPortHandle pxOut;

	pxOut = xSerialPortInitMinimal(0, 38400, 64);

	serial_putsz(pxOut, "\r\n[ALIVE]\r\n\r\n");

	for (;;) {
		result = xQueueReceive( xPacketQueue, outBuffer, QUEUE_TICKS);
		if (result == pdTRUE) {
			serial_putsz(pxOut, "[RCV] ");
			serial_putsz(pxOut, (char *)outBuffer);
			serial_putsz(pxOut, "\r\n");
		}

	}
}


void task_serial_test_start( UBaseType_t uxPriority )
{
	xPacketQueue = xQueueCreate( 10, BUFFER_SIZE );
	if (xPacketQueue == 0) {
		led_alert_on();
	}
	else {
		xTaskCreate( task_ble, "ble", configMINIMAL_STACK_SIZE, NULL, uxPriority,     ( TaskHandle_t * ) NULL );
		xTaskCreate( task_wan, "wan", configMINIMAL_STACK_SIZE, NULL, uxPriority + 1, ( TaskHandle_t * ) NULL );
	}
}
