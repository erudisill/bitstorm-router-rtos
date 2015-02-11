/*
 * led.c
 *
 *  Created on: Feb 4, 2015
 *      Author: jcobb
 */

#include <avr/io.h>
#include "led.h"

#define	LED_RED_bv		(_BV(PD6))
#define LED_GREEN_bv	(_BV(PD7))

void led_init(void)
{
	DDRD |= LED_RED_bv; 	// red (D3) alert
	PORTD |= LED_RED_bv;	// set led red off

	DDRD |= LED_GREEN_bv; 	// green (D2) output
	PORTD |= LED_GREEN_bv; 	// set led green off
}

void led_toggle(void)
{
	PORTD ^= LED_GREEN_bv;
}

void led_alert_on(void) {
	PORTD &= ~LED_RED_bv;
}

void led_alert_off(void) {
	PORTD |= LED_RED_bv;
}

void led_alert_toggle(void) {
	PORTD ^= LED_RED_bv;
}
