/*
 FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
 All rights reserved

 VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

 This file is part of the FreeRTOS distribution.

 FreeRTOS is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License (version 2) as published by the
 Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

 ***************************************************************************
 >>!   NOTE: The modification to the GPL is included to allow you to     !<<
 >>!   distribute a combined work that includes FreeRTOS without being   !<<
 >>!   obliged to provide the source code for proprietary components     !<<
 >>!   outside of the FreeRTOS kernel.                                   !<<
 ***************************************************************************

 FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  Full license text is available on the following
 link: http://www.freertos.org/a00114.html

 ***************************************************************************
 *                                                                       *
 *    FreeRTOS provides completely free yet professionally developed,    *
 *    robust, strictly quality controlled, supported, and cross          *
 *    platform software that is more than just the market leader, it     *
 *    is the industry's de facto standard.                               *
 *                                                                       *
 *    Help yourself get started quickly while simultaneously helping     *
 *    to support the FreeRTOS project by purchasing a FreeRTOS           *
 *    tutorial book, reference manual, or both:                          *
 *    http://www.FreeRTOS.org/Documentation                              *
 *                                                                       *
 ***************************************************************************

 http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
 the FAQ page "My application does not run, what could be wrong?".  Have you
 defined configASSERT()?

 http://www.FreeRTOS.org/support - In return for receiving this top quality
 embedded software for free we request you assist our global community by
 participating in the support forum.

 http://www.FreeRTOS.org/training - Investing in training allows your team to
 be as productive as possible as early as possible.  Now you can receive
 FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
 Ltd, and the world's leading authority on the world's leading RTOS.

 http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
 including FreeRTOS+Trace - an indispensable productivity tool, a DOS
 compatible FAT file system, and our tiny thread aware UDP/IP stack.

 http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
 Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

 http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
 Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
 licenses offer ticketed support, indemnification and commercial middleware.

 http://www.SafeRTOS.com - High Integrity Systems also provide a safety
 engineered and independently SIL3 certified version for use in safety and
 mission critical applications that require provable dependability.

 1 tab == 4 spaces!
 */

/*
 Changes from V1.2.3

 + The function xPortInitMinimal() has been renamed to
 xSerialPortInitMinimal() and the function xPortInit() has been renamed
 to xSerialPortInit().

 Changes from V2.0.0

 + Delay periods are now specified using variables and constants of
 TickType_t rather than unsigned long.
 + xQueueReceiveFromISR() used in place of xQueueReceive() within the ISR.

 Changes from V2.6.0

 + Replaced the inb() and outb() functions with direct memory
 access.  This allows the port to be built with the 20050414 build of
 WinAVR.
 */

/* BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER. */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "serial.h"

#define serBAUD_DIV_CONSTANT			( ( unsigned long ) 16 )

/* Constants for writing to UCSRB. */
#define serRX_INT_ENABLE				( ( unsigned char ) _BV(RXCIE0) )
#define serRX_ENABLE					( ( unsigned char ) _BV(RXEN0) )
#define serTX_ENABLE					( ( unsigned char ) _BV(TXEN0) )
#define serTX_INT_ENABLE				( ( unsigned char ) _BV(UDRIE0) )

/* Constants for writing to UCSRC. */
#define serEIGHT_DATA_BITS				( ( unsigned char ) (_BV(UCSZ01) | _BV(UCSZ00)) )

typedef struct {
	unsigned char ucPort;
	QueueHandle_t xRxedChars;
	QueueHandle_t xCharsForTx;
} ComPort_t;

static ComPort_t xaPorts[2];

#define vInterruptOn_0()										\
{															\
	unsigned char ucByte;								\
															\
	ucByte = UCSR0B;											\
	ucByte |= serTX_INT_ENABLE;								\
	UCSR0B = ucByte;											\
}																				
/*-----------------------------------------------------------*/

#define vInterruptOff_0()										\
{															\
	unsigned char ucInByte;								\
															\
	ucInByte = UCSR0B;										\
	ucInByte &= ~serTX_INT_ENABLE;							\
	UCSR0B = ucInByte;										\
}
#define vInterruptOn_1()										\
{															\
	unsigned char ucByte;								\
															\
	ucByte = UCSR1B;											\
	ucByte |= serTX_INT_ENABLE;								\
	UCSR1B = ucByte;											\
}
/*-----------------------------------------------------------*/

#define vInterruptOff_1()										\
{															\
	unsigned char ucInByte;								\
															\
	ucInByte = UCSR1B;										\
	ucInByte &= ~serTX_INT_ENABLE;							\
	UCSR1B = ucInByte;										\
}

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal(unsigned char ucPort, unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength) {
	unsigned long ulBaudRateCounter;
	unsigned char ucByte;
	ComPort_t * xComPort;

	portENTER_CRITICAL()
	;
	{
		/* Grab the pointer to the ComPort structure */
		xComPort = &xaPorts[ucPort];
		xComPort->ucPort = ucPort;

		/* Create the queues used by the com test task. */
		xComPort->xRxedChars = xQueueCreate(uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ));
		xComPort->xCharsForTx = xQueueCreate(uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ));

		/* Calculate the baud rate register value from the equation in the data sheet. */
		ulBaudRateCounter = ( configCPU_CLOCK_HZ / ( serBAUD_DIV_CONSTANT * ulWantedBaud)) - (unsigned long) 1;

		/* Simple if statement works here for now. Maybe get clever with pointers later. */
		if (ucPort == 0) {
			/* Set the baud rate. */
			ucByte = (unsigned char) (ulBaudRateCounter & (unsigned long) 0xff);
			UBRR0L = ucByte;

			ulBaudRateCounter >>= (unsigned long) 8;
			ucByte = (unsigned char) (ulBaudRateCounter & (unsigned long) 0xff);
			UBRR0H = ucByte;

			/* Enable the Rx interrupt.  The Tx interrupt will get enabled
			 later. Also enable the Rx and Tx. */
			UCSR0B = ( serRX_INT_ENABLE | serRX_ENABLE | serTX_ENABLE);

			/* Set the data bits to 8. */
			UCSR0C = ( serEIGHT_DATA_BITS);
		} else {
			/* Set the baud rate. */
			ucByte = (unsigned char) (ulBaudRateCounter & (unsigned long) 0xff);
			UBRR1L = ucByte;

			ulBaudRateCounter >>= (unsigned long) 8;
			ucByte = (unsigned char) (ulBaudRateCounter & (unsigned long) 0xff);
			UBRR1H = ucByte;

			/* Enable the Rx interrupt.  The Tx interrupt will get enabled
			 later. Also enable the Rx and Tx. */
			UCSR1B = ( serRX_INT_ENABLE | serRX_ENABLE | serTX_ENABLE);

			/* Set the data bits to 8. */
			UCSR1C = ( serEIGHT_DATA_BITS);
		}
	}
	portEXIT_CRITICAL()
	;

	return xComPort;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar(xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime) {

	/* Get the next character from the buffer.  Return false if no characters are available, or arrive before xBlockTime expires. */
	if (xQueueReceive(((ComPort_t*)pxPort)->xRxedChars, pcRxedChar, xBlockTime)) {
		return pdTRUE;
	} else {
		return pdFALSE;
	}
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar(xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime) {

	/* Return false if after the block time there is no room on the Tx queue. */
	if ( xQueueSend( ((ComPort_t*)pxPort)->xCharsForTx, &cOutChar, xBlockTime ) != pdPASS) {
		return pdFAIL;
	}

	if (((ComPort_t*)pxPort)->ucPort == 0) {
		vInterruptOn_0();
	}
	else {
		vInterruptOn_1();
	}

	return pdPASS;
}
/*-----------------------------------------------------------*/

void vSerialClose(xComPortHandle xPort) {
	unsigned char ucByte;

	/* Turn off the interrupts.  We may also want to delete the queues and/or re-install the original ISR. */

	portENTER_CRITICAL()
	;
	{
		if (((ComPort_t*)xPort)->ucPort == 0) {
			vInterruptOff_0();
			ucByte = UCSR0B;
			ucByte &= ~serRX_INT_ENABLE;
			UCSR0B = ucByte;
		}
		else {
			vInterruptOff_1();
			ucByte = UCSR1B;
			ucByte &= ~serRX_INT_ENABLE;
			UCSR1B = ucByte;
		}
	}
	portEXIT_CRITICAL()
	;
}
/*-----------------------------------------------------------*/

ISR( USART0_RX_vect ) {
	signed char cChar;
	signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Get the character and post it on the queue of Rxed characters.
	 If the post causes a task to wake force a context switch as the woken task
	 may have a higher priority than the task we have interrupted. */
	cChar = UDR0;

	xQueueSendFromISR(xaPorts[0].xRxedChars, &cChar, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken != pdFALSE) {
		taskYIELD();
	}
}
ISR( USART1_RX_vect ) {
	signed char cChar;
	signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Get the character and post it on the queue of Rxed characters.
	 If the post causes a task to wake force a context switch as the woken task
	 may have a higher priority than the task we have interrupted. */
	cChar = UDR1;

	xQueueSendFromISR(xaPorts[1].xRxedChars, &cChar, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken != pdFALSE) {
		taskYIELD();
	}
}
/*-----------------------------------------------------------*/

ISR( USART0_UDRE_vect ) {
	signed char cChar, cTaskWoken;

	if (xQueueReceiveFromISR(xaPorts[0].xCharsForTx, &cChar, &cTaskWoken) == pdTRUE) {
		/* Send the next character queued for Tx. */
		UDR0 = cChar;
	} else {
		/* Queue empty, nothing to send. */
		vInterruptOff_0();
	}
}
ISR( USART1_UDRE_vect ) {
	signed char cChar, cTaskWoken;

	if (xQueueReceiveFromISR(xaPorts[1].xCharsForTx, &cChar, &cTaskWoken) == pdTRUE) {
		/* Send the next character queued for Tx. */
		UDR1 = cChar;
	} else {
		/* Queue empty, nothing to send. */
		vInterruptOff_0();
	}
}


