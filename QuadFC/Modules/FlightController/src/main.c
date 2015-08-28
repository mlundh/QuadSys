/*
 * main.c
 *
 * Copyright (C) 2014  Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <sysclk.h>
#include <board.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/*Include units*/
#include "main_control_task.h"
#include "satellite_receiver_task.h"
#include "communication_tasks.h"
#include "led_control_task.h"
#include "range_meter.h"

#include "globals.h"
/*
 * FreeRTOS hook (or callback) functions that are defined in this file.
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask,
    signed char *pcTaskName );
void vApplicationTickHook( void );

 QueueHandle_t xQueue_receiver = NULL;
 QueueHandle_t xQueue_ranger = NULL;
SemaphoreHandle_t x_param_mutex = NULL;
SemaphoreHandle_t x_log_mutex = NULL;
/*
 * Set up the hardware to run QuadFC.
 */
static void prvSetupHardware( void );

int main( void )
{

  /* Prepare the hardware to run QuadFC. */
  prvSetupHardware();

  /* Create the queue used for receiver communication*/
  xQueue_receiver = xQueueCreate( RECEIVER_QUEUE_LENGTH,
      RECEIVER_QUEUE_ITEM_SIZE );

  /*Create the queue used to pass new range data to the main task*/
  xQueue_ranger = xQueueCreate( RANGER_QUEUE_LENGTH, RANGER_QUEUE_ITEM_SIZE );
  /* Create the queue used to set the LED status*/


  x_param_mutex = xSemaphoreCreateMutex();
  x_log_mutex = xSemaphoreCreateMutex();



  if ( !xQueue_receiver || !x_param_mutex || !x_log_mutex || !xQueue_ranger)
  {
    /*If one of thee queues could not be created, do nothing*/
    for ( ;; )
    {
    }
  }
  /* Create all tasks used in the application.*/
  create_satellite_receiver_task();
  Led_CreateLedControlTask();
  Com_CreateTasks(); // Creates two tasks, RX and TX.
  create_range_meter_task();
  create_main_control_task();
  /* Start the RTOS scheduler. */
  vTaskStartScheduler();
  /* If all is well, the scheduler will now be running, and the following line
   will never be reached.  If the following line does execute, then there was
   insufficient FreeRTOS heap memory available for the idle and/or timer tasks
   to be created.  See the memory management section on the FreeRTOS web site
   for more details. */
  for ( ;; )
  {
  }
  return 0;
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
  /* ASF function to setup clocking. */
  sysclk_init();

  /* Ensure all priority bits are assigned as preemption priority bits. */
  NVIC_SetPriorityGrouping( 0 );

  /* Atmel library function to setup for the evaluation kit being used. */
  board_init();

}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
  /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created.  It is also called by various parts of the
   demo application.  If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
  taskDISABLE_INTERRUPTS();
  for ( ;; )
  {
  }
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
  /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
   task.  It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()).  If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */

}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask,
    signed char *pcTaskName )
{
  (void) pcTaskName;
  (void) pxTask;

  /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
   function is called if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  for ( ;; )
  {
  }
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
  /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}

/*-----------------------------------------------------------*/

void assert_triggered( const char *file, uint32_t line )
{
  volatile uint32_t block_var = 0, line_in;
  const char *file_in;

  /* These assignments are made to prevent the compiler optimizing the
   values away. */
  file_in = file;
  line_in = line;
  (void) file_in;
  (void) line_in;

  taskENTER_CRITICAL();
  {
    while ( block_var == 0 )
    {
      /* Set block_var to a non-zero value in the debugger to
       step out of this function. */
    }
  }
  taskEXIT_CRITICAL();
}
