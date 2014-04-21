/**
 * \file
 *
 * \brief Arduino Due/X Board Definition.
 *
 * Copyright (c) 2011 - 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef ARDUINO_DUE_X_H_INCLUDED
#define ARDUINO_DUE_X_H_INCLUDED

#include "compiler.h"
#include "system_sam3x.h"
#include "exceptions.h"

/* ------------------------------------------------------------------------ */

/**
 *  \page arduino_due_x_opfreq "Arduino Due/X - Operating frequencies"
 *  This page lists several definition related to the board operating frequency
 *
 *  \section Definitions
 *  - \ref BOARD_FREQ_*
 *  - \ref BOARD_MCK
 */

/*! Board oscillator settings */
#define BOARD_FREQ_SLCK_XTAL            (32768U)
#define BOARD_FREQ_SLCK_BYPASS          (32768U)
#define BOARD_FREQ_MAINCK_XTAL          (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS        (12000000U)

/*! Master clock frequency */
#define BOARD_MCK                       CHIP_FREQ_CPU_MAX
#define BOARD_NO_32K_XTAL

/** board main clock xtal statup time */
#define BOARD_OSC_STARTUP_US   15625

/* ------------------------------------------------------------------------ */

/**
 * \page arduino_due_x_board_info "Arduino Due/X - Board informations"
 * This page lists several definition related to the board description.
 *
 * \section Definitions
 * - \ref BOARD_NAME
 */

/*! Name of the board */
#define BOARD_NAME "Arduino Due/X"
/*! Board definition */
#define arduinoduex
/*! Family definition (already defined) */
#define sam3x
/*! Core definition */
#define cortexm3

/* ------------------------------------------------------------------------ */

/**
 * \page arduino_due_x_piodef "Arduino Due/X - PIO definitions"
 * This pages lists all the pio definitions. The constants
 * are named using the following convention: PIN_* for a constant which defines
 * a single Pin instance (but may include several PIOs sharing the same
 * controller), and PINS_* for a list of Pin instances.
 *
 */

/**
 * \file
 * LEDs
 *
 */

/* ------------------------------------------------------------------------ */
/* PINs                                                                     */
/* ------------------------------------------------------------------------ */
/*! Power LED pin definition (ORANGE). L */
#define PIN_POWER_LED   {PIO_PB27, PIOB, ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
/*! PIN #31 pin definition */
#define PIN_31_USER   {PIO_PA7, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
/*! LED #32 pin definition */
#define PIN_33_USER   {PIO_PC1, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
/*! LED #33 pin definition */
#define PIN_35_USER   {PIO_PC3, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}

/*! List of all LEDs definitions. */
#define PINS_LEDS   PIN_31_USER, PIN_33_USER, PIN_35_USER, PIN_POWER_LED

/*! LED #0 "L" pin definition (ORANGE).*/
#define LED_0_NAME      "Orange_LED"
#define LED0_GPIO       (PIO_PB27_IDX)
#define LED0_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define LED0_ACTIVE_LEVEL 0

#define PIN_LED_0       {1 << 27, PIOB, ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_LED_0_MASK  (1 << 27)
#define PIN_LED_0_PIO   PIOB
#define PIN_LED_0_ID    ID_PIOB
#define PIN_LED_0_TYPE  PIO_OUTPUT_0
#define PIN_LED_0_ATTR  PIO_DEFAULT

/*! PIN_31 pin definition */
#define PIN_31_GPIO       (PIO_PA7_IDX)
#define PIN_31_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define PIN_31_ACTIVE_LEVEL 1

#define PIN_31       {1 << 7, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_31_MASK  (1 << 7)
#define PIN_31_PIO   PIOA
#define PIN_31_ID    ID_PIOA
#define PIN_31_TYPE  PIO_OUTPUT_1
#define PIN_31_ATTR  PIO_DEFAULT

/*! PIN_33 pin definition */
#define PIN_33_GPIO       (PIO_PC1_IDX)
#define PIN_33_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define PIN_33_ACTIVE_LEVEL 1

#define PIN_33       {1 << 1, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_33_MASK  (1 << 1)
#define PIN_33_PIO   PIOC
#define PIN_33_ID    ID_PIOC
#define PIN_33_TYPE  PIO_OUTPUT_1
#define PIN_33_ATTR  PIO_DEFAULT

/*! PIN_35 pin definition */
#define PIN_35_GPIO       (PIO_PC3_IDX)
#define PIN_35_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define PIN_35_ACTIVE_LEVEL 1

#define PIN_35	     {1 << 3, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_35_MASK  (1 << 3)
#define PIN_35_PIO   PIOC
#define PIN_35_ID    ID_PIOC
#define PIN_35_TYPE  PIO_OUTPUT_1
#define PIN_35_ATTR  PIO_DEFAULT

/*! PIN_37 pin definition */
#define PIN_37_GPIO       (PIO_PC5_IDX)
#define PIN_37_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define PIN_37_ACTIVE_LEVEL 1

#define PIN_37	     {1 << 5, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_37_MASK  (1 << 5)
#define PIN_37_PIO   PIOC
#define PIN_37_ID    ID_PIOC
#define PIN_37_TYPE  PIO_OUTPUT_1
#define PIN_37_ATTR  PIO_DEFAULT

/*! PIN_35 pin definition */
#define PIN_39_GPIO       (PIO_PC7_IDX)
#define PIN_39_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define PIN_39_ACTIVE_LEVEL 1

#define PIN_39	     {1 << 7, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_39_MASK  (1 << 7)
#define PIN_39_PIO   PIOC
#define PIN_39_ID    ID_PIOC
#define PIN_39_TYPE  PIO_OUTPUT_1
#define PIN_39_ATTR  PIO_DEFAULT

/*! PIN_35 pin definition */
#define PIN_41_GPIO       (PIO_PC9_IDX)
#define PIN_41_FLAGS      (PIO_TYPE_PIO_OUTPUT_0 | PIO_DEFAULT)
#define PIN_41_ACTIVE_LEVEL 1

#define PIN_41	     {1 << 9, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_41_MASK  (1 << 9)
#define PIN_41_PIO   PIOC
#define PIN_41_ID    ID_PIOC
#define PIN_41_TYPE  PIO_OUTPUT_1
#define PIN_41_ATTR  PIO_DEFAULT


/**
 * \file
 * PWMC
 * - \ref PIN_PWMC_PWMH0
 * - \ref PIN_PWMC_PWML4
 * - \ref PIN_PWMC_PWML5
 * - \ref PIN_PWMC_PWML6
 * - \ref PIN_PWM_LED0
 * - \ref PIN_PWM_LED1
 * - \ref PIN_PWM_LED2
 *
 */

/* ------------------------------------------------------------------------ */
/* PWM                                                                      */
/* ------------------------------------------------------------------------ */

/*! PWM "PWM6" pin definitions.*/
#define PIN_6_PWM_GPIO    PIO_PC24_IDX
#define PIN_6_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_6_PWM_CHANNEL PWM_CHANNEL_7

/*! PWM "PWM7" pin definitions.*/
#define PIN_7_PWM_GPIO    PIO_PC23_IDX
#define PIN_7_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_7_PWM_CHANNEL PWM_CHANNEL_6

/*! PWM "PWM8" pin definitions.*/
#define PIN_8_PWM_GPIO    PIO_PC22_IDX
#define PIN_8_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_8_PWM_CHANNEL PWM_CHANNEL_5

/*! PWM "PWM9" pin 9 definitions.*/
#define PIN_9_PWM_GPIO    PIO_PC21_IDX
#define PIN_9_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_9_PWM_CHANNEL PWM_CHANNEL_4

/*! PWM on pin 34 pin definitions.*/
#define PIN_34_PWM_GPIO    PIO_PC2_IDX
#define PIN_34_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_34_PWM_CHANNEL PWM_CHANNEL_0

/*! PWM on pin 36 pin definitions.*/
#define PIN_36_PWM_GPIO    PIO_PC4_IDX
#define PIN_36_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_36_PWM_CHANNEL PWM_CHANNEL_1

/*! PWM on pin 38 pin definitions.*/
#define PIN_38_PWM_GPIO    PIO_PC6_IDX
#define PIN_38_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_38_PWM_CHANNEL PWM_CHANNEL_2

/*! PWM on pin 40 pin definitions.*/
#define PIN_40_PWM_GPIO    PIO_PC8_IDX
#define PIN_40_PWM_FLAGS   (PIO_PERIPH_B | PIO_DEFAULT)
#define PIN_40_PWM_CHANNEL PWM_CHANNEL_3



/**
 * \file
 * USART0
 * - \ref PIN_USART0_RXD
 * - \ref PIN_USART0_TXD
 */
/* ------------------------------------------------------------------------ */
/* USART0                                                                   */
/* ------------------------------------------------------------------------ */
/*! USART0 pin RX  (labeled RX1 19)*/
#define PIN_USART0_RXD\
	{PIO_PA10A_RXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_RXD_IDX        (PIO_PA10_IDX)
#define PIN_USART0_RXD_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)

/*! USART0 pin TX  (labeled TX1 18) */
#define PIN_USART0_TXD\
	{PIO_PA11A_TXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART0_TXD_IDX        (PIO_PA11_IDX)
#define PIN_USART0_TXD_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)

/**
 * \file
 * USART1
 * - \ref PIN_USART1_RXD
 * - \ref PIN_USART1_TXD
 */
/* ------------------------------------------------------------------------ */
/* USART1                                                                   */
/* ------------------------------------------------------------------------ */
/*! USART1 pin RX (labeled RX2 17) */
#define PIN_USART1_RXD\
	{PIO_PA12A_RXD1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_RXD_IDX        (PIO_PA12_IDX)
#define PIN_USART1_RXD_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
/*! USART1 pin TX (labeled TX2 16) */
#define PIN_USART1_TXD\
	{PIO_PA13A_TXD1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_TXD_IDX        (PIO_PA13_IDX)
#define PIN_USART1_TXD_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)

/**
 * \file
 * USART3
 * - \ref PIN_USART3_RXD
 * - \ref PIN_USART3_TXD
 */

/* ------------------------------------------------------------------------ */
/* USART3                                                                   */
/* ------------------------------------------------------------------------ */
/*! USART3 pin RX (labeled RX3 15) */
#define PIN_USART3_RXD\
	{PIO_PD5B_RXD3, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_USART3_RXD_IDX        (PIO_PD5_IDX)
#define PIN_USART3_RXD_FLAGS      (PIO_PERIPH_B | PIO_DEFAULT)
/*! USART3 pin TX (labeled RX3 14) */
#define PIN_USART3_TXD\
	{PIO_PD4B_TXD3, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_USART3_TXD_IDX        (PIO_PD4_IDX)
#define PIN_USART3_TXD_FLAGS      (PIO_PERIPH_B | PIO_DEFAULT)


/**
 * \file
 * TWI
 */
/* ------------------------------------------------------------------------ */
/* TWI                                                                      */
/* ------------------------------------------------------------------------ */
/*! TWI0 pins definition */
#define TWI0_DATA_GPIO   PIO_PA17_IDX
#define TWI0_DATA_FLAGS  (PIO_PERIPH_A | PIO_DEFAULT)
#define TWI0_CLK_GPIO    PIO_PA18_IDX
#define TWI0_CLK_FLAGS   (PIO_PERIPH_A | PIO_DEFAULT)

/*! TWI1 pins definition */
#define TWI1_DATA_GPIO   PIO_PB12_IDX
#define TWI1_DATA_FLAGS  (PIO_PERIPH_A | PIO_DEFAULT)
#define TWI1_CLK_GPIO    PIO_PB13_IDX
#define TWI1_CLK_FLAGS   (PIO_PERIPH_A | PIO_DEFAULT)

#endif /* ARDUINO_DUE_X_H_INCLUDED */
