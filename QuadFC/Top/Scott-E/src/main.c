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
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/*Include board*/
#include "HAL/QuadFC/QuadFC_Board.h"
#include "BoardConfig.h"
/*Include tasks to start*/
#include "Modules/Scott-E/inc/Scott-E_MainTask.h"
#include "Communication/inc/communication_tasks.h"
#include "HMI/inc/led_control_task.h"
#include "SpectrumSatellite/inc/satellite_receiver_public.h"
#include "ServiceTask/inc/ServiceTask.h"
#include "AppLogTask/inc/AppLogTask.h"
/*include utilities*/
#include "Messages/inc/common_types.h"

/*Include task communication modules*/
#include "Components/ControlModeHandler/inc/control_mode_handler.h"
#include "EventHandler/inc/event_handler.h"

/*
 * FreeRTOS hook (or callback) functions that are defined in this file.
 */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask,
                                   signed char *pcTaskName);
void vApplicationTickHook(void);

int main(void)
{

    /* Prepare the hardware to run QuadFC. */
    Board_SetupHardware();

    /* Create all handlers used in the application.*/
    eventHandler_t *evHandlerM = Event_CreateHandler(FC_Ctrl_e, 1);
    eventHandler_t *evHandlerSatelliteInternal = Event_CreateHandler(RC_SetpointGen_e, 0);
    eventHandler_t *evHandlerSatelliteExternal = Event_CreateHandler(RC_SetpointGen2_e, 0);

    eventHandler_t *evHandlerComRx = Event_CreateHandler(FC_SerialIOrx_e, 0);
    eventHandler_t *evHandlerComTx = Event_CreateHandler(FC_SerialIOtx_e, 0);

    eventHandler_t *evHandlerLed = Event_CreateHandler(FC_Led_e, 0);

    eventHandler_t *evHandlerService = Event_CreateHandler(FC_Param_e, 0);
    eventHandler_t *evHandlerAppLog = Event_CreateHandler(FC_AppLog_e, 0);

    if (!evHandlerM || !evHandlerComRx || !evHandlerLed || !evHandlerSatelliteInternal || !evHandlerSatelliteExternal || !evHandlerService || !evHandlerAppLog)
    {
        for (;;)
            ; // Error!
    }

    Scott_e_CreateTask(evHandlerM, EXT_2_SERIAL_BUS);
    Satellite_CreateReceiverTask(evHandlerSatelliteInternal, RC1_SERIAL_BUS, Gpio_Rc1PwrCtrl, '1');
    Satellite_CreateReceiverTask(evHandlerSatelliteExternal, RC2_SERIAL_BUS, Gpio_Rc1PwrCtrl, '2');
    Led_CreateLedControlTask(evHandlerLed);
    Com_CreateTasks(evHandlerComRx, evHandlerComTx, COM_SERIAL_BUS, COM_WIRELESS_SERIAL_BUS); // Creates two tasks, RX and TX.

    Service_CreateTask(evHandlerService);
    AppLog_CreateTask(evHandlerAppLog);

    Event_InitHandler(evHandlerM, evHandlerComRx);
    Event_InitHandler(evHandlerM, evHandlerComTx);
    Event_InitHandler(evHandlerM, evHandlerLed);
    Event_InitHandler(evHandlerM, evHandlerSatelliteInternal);
    Event_InitHandler(evHandlerM, evHandlerSatelliteExternal);
    Event_InitHandler(evHandlerM, evHandlerService);
    Event_InitHandler(evHandlerM, evHandlerAppLog);

    /* Start the RTOS scheduler. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following line
     will never be reached.  If the following line does execute, then there was
     insufficient FreeRTOS heap memory available for the idle and/or timer tasks
     to be created.  See the memory management section on the FreeRTOS web site
     for more details. */
    for (;;)
    {
    }
    return 0;
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
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
    for (;;)
    {
    }
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
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

void vApplicationStackOverflowHook(TaskHandle_t pxTask,
                                   signed char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
     configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
    /* This function will be called by each tick interrupt if
     configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
     added here, but the tick hook is called from an interrupt context, so
     code must not attempt to block, and only the interrupt safe FreeRTOS API
     functions can be used (those that end in FromISR()). */
}

/*-----------------------------------------------------------*/

void assert_triggered(const char *file, uint32_t line)
{
    volatile uint32_t block_var = 0, line_in;
    const char *file_in;

    /* These assignments are made to prevent the compiler optimizing the
     values away. */
    file_in = file;
    line_in = line;
    (void)file_in;
    (void)line_in;

    taskENTER_CRITICAL();
    {
        while (block_var == 0)
        {
            /* Set block_var to a non-zero value in the debugger to
             step out of this function. */
        }
    }
    taskEXIT_CRITICAL();
}

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
void HardFault_Handler(void)
{
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n");
}

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
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
    volatile uint32_t lr = 0;  /* Link register. */
    volatile uint32_t pc = 0;  /* Program counter. */
    volatile uint32_t psr = 0; /* Program status register. */

    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;
    (void)r12;
    (void)lr;
    (void)pc;
    (void)psr;

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    /* When the following line is hit, the variables contain the register values. */
    for (;;)
        ;
}
