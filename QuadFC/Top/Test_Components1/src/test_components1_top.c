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
#include "Test/Parameters/parameter_tester.h"
#include "Test/Log/log_tester.h"
#include "Test/Log/logEventTester.h"
#include "Test/AppLog/AppLogTest.h"
#include "Test/CharCircularBuffer/cbuff_tester.h"

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

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
//static void HardFault_Handler( void ) __attribute__( ( naked ) );

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress );

/*
 * FreeRTOS hook (or callback) functions that are defined in this file.
 */
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask,
        signed char *pcTaskName );

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
    TestFw_t* testFW = TestFW_Create("Components1");

    /**************Add test module instantiation here***************/
    ParamT_GetTCs(testFW);
    Log_GetTCs(testFW);
    LogEv_GetTCs(testFW);
    AppLog_GetTCs(testFW);
    Cbuff_GetTCs(testFW);
    /***************************************************************/

    uint8_t result = TestFW_ExecuteTests(testFW);

    uint32_t pin = (result ? ledHeartBeat : ledFatal);
    while ( 1 )
    {
        Led_Toggle( pin );
        vTaskDelay(1000 / portTICK_PERIOD_MS );
    }
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

