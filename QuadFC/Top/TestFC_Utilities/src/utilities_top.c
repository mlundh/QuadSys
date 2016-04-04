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
#include <sysclk.h>
#include <board.h>
#include <gpio.h>
#include <pio.h>
//Include testers
#include "Test/TestFW/test_framework.h"
#include "Test/Math/math_tester.h"

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

/*
 * Set up the hardware to run QuadFC.
 */
static void prvSetupHardware( void );

int main( void )
{

  /* Prepare the hardware to run QuadFC. */
  prvSetupHardware();

  TestFw_t* testFW = TestFW_Create("Suite1");
  /**************Add test module instantiation here***************/
  MathTest_GetTCs(testFW);
  /***************************************************************/

  uint8_t result = TestFW_ExecuteTests(testFW);
  uint32_t heartbeat_counter = 0;

  uint32_t pin = (result ? PIN_31_GPIO : PIN_41_GPIO);
  while ( 1 )
  {
    heartbeat_counter++;
    if ( heartbeat_counter >= 1000000 )
    {
      gpio_toggle_pin( pin );
      heartbeat_counter = 0;
    }

  }
}

static void prvSetupHardware( void )
{
  /* ASF function to setup clocking. */
  sysclk_init();

  /* Ensure all priority bits are assigned as preemption priority bits. */
  NVIC_SetPriorityGrouping( 0 );

  /* Atmel library function to setup for the evaluation kit being used. */
  board_init();

}
