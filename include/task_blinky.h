/*
 * task_blinky.h
 *
 *  Created on: Feb 10, 2015
 *      Author: ericrudisill
 */

#ifndef INCLUDE_TASK_BLINKY_H_
#define INCLUDE_TASK_BLINKY_H_

#include "FreeRTOS.h"
#include "task.h"
#include "led.h"

void task_blinky_start( UBaseType_t uxPriority );


#endif /* INCLUDE_TASK_BLINKY_H_ */
