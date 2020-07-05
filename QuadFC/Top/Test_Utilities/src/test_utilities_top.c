/*
 * utilities_top.c
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
/*Include board*/
#include "HAL/QuadFC/QuadFC_Board.h"
#include "HAL/QuadFC/QuadFC_Gpio.h"
//Include testers
#include "Test/TestFW/test_framework.h"
#include "Test/Math/math_tester.h"
#include "Test/Utilities/utilities_tester.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/**
 * @file Top used for regression testing. This top does not include any
 * RTOS, and should therefore only test modules that does not depend on
 * any methods provided by freeRTOS.
 *
 * The test is supposed to run on the same hardware as the real software,
 * and should report a successful test by sending a message over the
 * serial port. The serial port will be handled by the test framework, and
 * communication over the port will use QSP frames.
 */

void vApplicationStackOverflowHook( TaskHandle_t pxTask,
    signed char *pcTaskName );

int main( void )
{

  /* Prepare the hardware to run QuadFC. */
  Board_SetupHardware();


  TestFw_t* testFW = TestFW_Create("Math and Util");
  /**************Add test module instantiation here***************/
  MathTest_GetTCs(testFW);
  Utilities_GetTCs(testFW);
  /***************************************************************/

  uint8_t result = TestFW_ExecuteTests(testFW);
  uint32_t heartbeat_counter = 0;
  uint32_t pin = (result ? ledHeartBeat : ledFatal);
  while ( 1 )
  {
    heartbeat_counter++;
    if ( heartbeat_counter >= 1000000 )
    {
      Gpio_TogglePin( pin );
      heartbeat_counter = 0;
    }

  }
}


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
