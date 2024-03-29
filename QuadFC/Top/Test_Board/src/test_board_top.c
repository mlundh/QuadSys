/*
 * dummy_rtos_top.c
 *
 * Copyright (C) 2016 Martin Lundh
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

#include "HAL/QuadFC/QuadFC_Board.h"
#include "HAL/QuadFC/QuadFC_Gpio.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
//Include testers
#include "Test/TestFW/test_framework.h"
#include "HMI/inc/led_control_task.h"
#include "Test/ArduinoDueBoard/board_test.h"
#include "HAL/QuadFC/QuadFC_Memory.h"

/**
 * @file Top used for regression testing. This top uses freeRTOS and
 * all of its features.
 *
 * The test is supposed to run on the same hardware as the real software,
 * and should report a successful test by sending a message over the
 * serial port. The serial port will be handled by the test framework, and
 * communication over the port will use QSP frames.
 */

void mainTester(void *pvParameters);

/*
 * FreeRTOS hook (or callback) functions that are defined in this file.
 */
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, signed char *pcTaskName );

int main( void )
{

  /* Prepare the hardware to run QuadFC. */
  Board_SetupHardware();
  TestFW_CreateMainTestTask(mainTester);
  /* Start the RTOS scheduler. */
  vTaskStartScheduler();
  for ( ;; )
  {
  }
  return 0;
}

void mainTester(void *pvParameters)
{
  TestFw_t* testFW = TestFW_Create("Peripherals");
  Gpio_Init(HEARTBEAT, Gpio_OutputOpenDrain, Gpio_NoPull);
  //Mem_Init();
  /**************Add test module instantiation here***************/
  Board_GetTCs(testFW);

  /***************************************************************/

  uint8_t result = TestFW_ExecuteTests(testFW);
  uint32_t heartbeat_counter = 0;

  uint32_t pin = (result ? ledHeartBeat : ledFatal);
  taskENTER_CRITICAL();


  while ( 1 )
  {
    heartbeat_counter++;
    if ( heartbeat_counter >= 1000000 )
    {
      Led_Toggle( pin );
      heartbeat_counter = 0;
    }
  }
  taskEXIT_CRITICAL();

}

void vApplicationMallocFailedHook( void )
{
  taskDISABLE_INTERRUPTS();
  for ( ;; )
  {
  }
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask,
    signed char *pcTaskName )
{
  (void) pcTaskName;
  (void) pxTask;

  taskDISABLE_INTERRUPTS();
  for ( ;; )
  {
  }
}




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

