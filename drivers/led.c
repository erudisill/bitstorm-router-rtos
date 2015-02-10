/*
 * led.c
 *
 *  Created on: Feb 4, 2015
 *      Author: jcobb
 */

#include <avr/io.h>
#include "led.h"

void led_init(void)
{
	DDRD |= _BV(PD7); // green (D2) output
	PORTD |= _BV(PD7); // set led green off
}

void led_toggle(void)
{
	PORTD ^= _BV(PD7);
}

