
#include "Boards/Titan/Titan_Utils/inc/watchpoint.h"


void watchpoint_enable(uint8_t watchpoint_index, uint32_t *word_address){
    CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk /*enable tracing*/ |
            CoreDebug_DEMCR_MON_EN_Msk /*enable debug interrupt*/;
 
    switch (watchpoint_index){
    case 0:
        DWT->COMP0 = (uint32_t)word_address;
        DWT->MASK0 = 0; //match all comparator bits, don't ignore any
        DWT->FUNCTION0 = (1 << 11)/*DATAVSIZE 1 - match whole word*/
                	| (1 << 1) | (1 << 2)/*generate a watchpoint event on write*/;
        break;
    case 1:
        DWT->COMP1 = (uint32_t)word_address;
        DWT->MASK1 = 0; //match all comparator bits, don't ignore any
        DWT->FUNCTION1 = (1 << 11)/*DATAVSIZE 1 - match whole word*/
                	| (1 << 1) | (1 << 2)/*generate a watchpoint event on write*/;
        break;
    case 2:
        DWT->COMP2 = (uint32_t)word_address;
        DWT->MASK2 = 0; //match all comparator bits, don't ignore any
        DWT->FUNCTION2 = (1 << 11)/*DATAVSIZE 1 - match whole word*/
                	| (1 << 1) | (1 << 2)/*generate a watchpoint event on write*/;
        break;
    }
}
 
void watchpoint_disable(uint8_t watchpoint_index){
    switch (watchpoint_index){
    case 0:
        DWT->FUNCTION0 = 0; //disable everything
        break;
    case 1:
        DWT->FUNCTION1 = 0; //disable everything
        break;
    case 2:
        DWT->FUNCTION2 = 0; //disable everything
        break;
    default:
        __BKPT(0); //there are only 3 hardware watchpoints!
    }
}

void DebugMon_HandlerC(unsigned long *args){
    /* Attribute unused is to silence compiler warnings,
     * the variables are placed here, so they can be inspected
     * by the debugger.
     */
    volatile unsigned long __attribute__((unused)) stacked_r0  = ((unsigned long)args[0]);
    volatile unsigned long __attribute__((unused)) stacked_r1  = ((unsigned long)args[1]);
    volatile unsigned long __attribute__((unused)) stacked_r2  = ((unsigned long)args[2]);
    volatile unsigned long __attribute__((unused)) stacked_r3  = ((unsigned long)args[3]);
    volatile unsigned long __attribute__((unused)) stacked_r12 = ((unsigned long)args[4]);
    volatile unsigned long __attribute__((unused)) stacked_lr  = ((unsigned long)args[5]);
    volatile unsigned long __attribute__((unused)) stacked_pc  = ((unsigned long)args[6]);
    volatile unsigned long __attribute__((unused)) stacked_psr = ((unsigned long)args[7]);
 
    // Configurable Fault Status Register
    // Consists of MMSR, BFSR and UFSR
    volatile unsigned long __attribute__((unused)) _CFSR = (*((volatile unsigned long *)(0xE000ED28))) ;
 
    // Hard Fault Status Register
    volatile unsigned long __attribute__((unused)) _HFSR = (*((volatile unsigned long *)(0xE000ED2C))) ;
 
    // Debug Fault Status Register
    volatile unsigned long __attribute__((unused)) _DFSR = (*((volatile unsigned long *)(0xE000ED30))) ;
 
    // Auxiliary Fault Status Register
    volatile unsigned long __attribute__((unused)) _AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;
 
    // Read the Fault Address Registers. These may not contain valid values.
    // Check BFARVALID/MMARVALID to see if they are valid values
    // MemManage Fault Address Register
    volatile unsigned long __attribute__((unused)) _MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
    // Bus Fault Address Register
    volatile unsigned long __attribute__((unused)) _BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;
 
    volatile uint8_t __attribute__((unused)) watchpoint_number = 0;
    if (DWT->FUNCTION0 & DWT_FUNCTION_MATCHED_Msk){
        watchpoint_number = 0;
    } else if (DWT->FUNCTION1 & DWT_FUNCTION_MATCHED_Msk){
        watchpoint_number = 1;
    } else if (DWT->FUNCTION2 & DWT_FUNCTION_MATCHED_Msk){
        watchpoint_number = 2;
    }
    for(;;);
    __BKPT(0); //data watchpoint!
    //TODO: save data to debugging breadcrumbs and reboot
}
 
extern void DebugMon_Handler(void);
__attribute__((naked)) void DebugMon_Handler(void){
    __asm volatile (
            " movs r0,#4       \n"
            " movs r1, lr      \n"
            " tst r0, r1       \n"
            " beq _MSP2         \n"
            " mrs r0, psp      \n"
            " b _HALT2          \n"
            "_MSP2:               \n"
            " mrs r0, msp      \n"
            "_HALT2:              \n"
            " ldr r1,[r0,#20]  \n"
            " b DebugMon_HandlerC \n"
    );
}