/*
 * main.h
 *
 * Created: 2013-03-24 15:31:25
 *  Author: Martin Lundh
 */ 


#ifndef MAIN_1_H_
#define MAIN_1_H_




/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* ASF includes. */
#include "sysclk.h"
#include "asf.h"

/*Include units*/
#include "main_control_task.h"
#include "satellite_receiver_task.h"
#include "communication_tasks.h"




/*
 * FreeRTOS hook (or callback) functions that are defined in this file.
 */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle pxTask,
		signed char *pcTaskName);
void vApplicationTickHook(void);

/*------------------------------------------------------------*/


#endif /* MAIN_1_H_ */
