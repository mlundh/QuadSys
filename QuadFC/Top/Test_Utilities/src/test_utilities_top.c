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
void vApplicationMallocFailedHook( void );

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
  uint32_t pin = (result ? HEARTBEAT : LED_FATAL);
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


/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
void HardFault_Handler(void)
{
    __asm volatile
    (
            " tst lr, #4                                                \n"
            " ite eq                                                    \n"
            " mrseq r0, msp                                             \n"
            " mrsne r0, psp                                             \n"
            " ldr r1, [r0, #24]                                         \n"
            " ldr r2, handler2_address_const                            \n"
            " bx r2                                                     \n"
            " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
    /* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
    volatile uint32_t r0 = 0;
    volatile uint32_t r1 = 0;
    volatile uint32_t r2 = 0;
    volatile uint32_t r3 = 0;
    volatile uint32_t r12 = 0;
    volatile uint32_t lr = 0; /* Link register. */
    volatile uint32_t pc = 0; /* Program counter. */
    volatile uint32_t psr = 0;/* Program status register. */

    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;
    (void)r12;
    (void)lr;
    (void)pc;
    (void)psr;

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

