/*
 * task_serial_test.h
 *
 *  Created on: Feb 10, 2015
 *      Author: ericrudisill
 */

#ifndef INCLUDE_TASK_SERIAL_TEST_H_
#define INCLUDE_TASK_SERIAL_TEST_H_

#include <stddef.h>

#include "FreeRTOS.h"
#include "task.h"
#include "serial.h"

void task_serial_test_start( UBaseType_t uxPriority );


#endif /* INCLUDE_TASK_SERIAL_TEST_H_ */
