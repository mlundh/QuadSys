/**
 * \file
 *
 * \brief Arduino Due/X board init.
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

#include "compiler.h"
#include "board.h"
#include "conf_board.h"
#include "gpio.h"

void board_init(void)
{
#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;
#endif

	/* Configure LED pins */
	gpio_configure_pin(LED0_GPIO, LED0_FLAGS);
	gpio_configure_pin(PIN_31_GPIO, PIN_31_FLAGS);
	gpio_configure_pin(PIN_33_GPIO, PIN_33_FLAGS);
	gpio_configure_pin(PIN_35_GPIO, PIN_35_FLAGS);
	gpio_configure_pin(PIN_37_GPIO, PIN_37_FLAGS);
	gpio_configure_pin(PIN_39_GPIO, PIN_39_FLAGS);
	gpio_configure_pin(PIN_41_GPIO, PIN_41_FLAGS);

#ifdef CONF_BOARD_PWM_PIN_6
/* Configure PWM LED0 pin */
	gpio_configure_pin(PIN_6_PWM_GPIO, PIN_6_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_7
/* Configure PWM LED0 pin */
	gpio_configure_pin(PIN_7_PWM_GPIO, PIN_7_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_8
/* Configure PWM LED1 pin */
	gpio_configure_pin(PIN_8_PWM_GPIO, PIN_8_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_9
/* Configure PWM LED2 pin */
	gpio_configure_pin(PIN_9_PWM_GPIO, PIN_9_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_34
/* Configure PWM LED0 pin */
	gpio_configure_pin(PIN_34_PWM_GPIO, PIN_34_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_36
/* Configure PWM LED0 pin */
	gpio_configure_pin(PIN_36_PWM_GPIO, PIN_36_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_38
/* Configure PWM LED0 pin */
	gpio_configure_pin(PIN_38_PWM_GPIO, PIN_38_PWM_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_PIN_40
/* Configure PWM LED0 pin */
	gpio_configure_pin(PIN_40_PWM_GPIO, PIN_40_PWM_FLAGS);
#endif

  /* Configure SPI0 pins */
#ifdef CONF_BOARD_SPI0
  gpio_configure_pin(SPI0_MISO_GPIO, SPI0_MISO_FLAGS);
  gpio_configure_pin(SPI0_MOSI_GPIO, SPI0_MOSI_FLAGS);
  gpio_configure_pin(SPI0_SPCK_GPIO, SPI0_SPCK_FLAGS);

  /**
   * For NPCS 1, 2, and 3, different PINs can be used to access the same
   * NPCS line.
   * Depending on the application requirements, the default PIN may not be
   * available.
   * Hence a different PIN should be selected using the
   * CONF_BOARD_SPI_NPCS_GPIO and
   * CONF_BOARD_SPI_NPCS_FLAGS macros.
   */

#   ifdef CONF_BOARD_SPI0_NPCS0
    gpio_configure_pin(SPI0_NPCS0_GPIO, SPI0_NPCS0_FLAGS);
#   endif

#   ifdef CONF_BOARD_SPI0_NPCS1
          gpio_configure_pin(SPI0_NPCS1_PA29_GPIO,SPI0_NPCS1_PA29_FLAGS);
#   endif
#endif // #ifdef CONF_BOARD_SPI0

  /* Configure SPI1 pins */
#ifdef CONF_BOARD_SPI1
  gpio_configure_pin(SPI1_MISO_GPIO, SPI1_MISO_FLAGS);
  gpio_configure_pin(SPI1_MOSI_GPIO, SPI1_MOSI_FLAGS);
  gpio_configure_pin(SPI1_SPCK_GPIO, SPI1_SPCK_FLAGS);

#   ifdef CONF_BOARD_SPI1_NPCS0
    gpio_configure_pin(SPI1_NPCS0_GPIO, SPI1_NPCS0_FLAGS);
#   endif

#   ifdef CONF_BOARD_SPI1_NPCS1
    gpio_configure_pin(SPI1_NPCS1_GPIO, SPI1_NPCS1_FLAGS);
#   endif

#   ifdef CONF_BOARD_SPI1_NPCS2
    gpio_configure_pin(SPI1_NPCS2_GPIO, SPI1_NPCS2_FLAGS);
#   endif

#   ifdef CONF_BOARD_SPI1_NPCS3
    gpio_configure_pin(SPI1_NPCS3_GPIO, SPI1_NPCS3_FLAGS);
#   endif
#endif


#ifdef CONF_BOARD_TWI0
	gpio_configure_pin(TWI0_DATA_GPIO, TWI0_DATA_FLAGS);
	gpio_configure_pin(TWI0_CLK_GPIO, TWI0_CLK_FLAGS);
#endif

#ifdef CONF_BOARD_TWI1
	gpio_configure_pin(TWI1_DATA_GPIO, TWI1_DATA_FLAGS);
	gpio_configure_pin(TWI1_CLK_GPIO, TWI1_CLK_FLAGS);
#endif

#ifdef CONF_BOARD_USART0_RXD
	/* Configure USART RXD pin */
	gpio_configure_pin(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
#endif

#ifdef CONF_BOARD_USART0_TXD
	/* Configure USART TXD pin */
	gpio_configure_pin(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);
#endif

#ifdef CONF_BOARD_USART1_RXD
/* Configure USART RXD pin */
gpio_configure_pin(PIN_USART1_RXD_IDX, PIN_USART1_RXD_FLAGS);
#endif

#ifdef CONF_BOARD_USART1_TXD
/* Configure USART TXD pin */
gpio_configure_pin(PIN_USART1_TXD_IDX, PIN_USART1_TXD_FLAGS);
#endif

#ifdef CONF_BOARD_USART3_RXD
/* Configure USART RXD pin */
gpio_configure_pin(PIN_USART3_RXD_IDX, PIN_USART3_RXD_FLAGS);
#endif

#ifdef CONF_BOARD_USART3_TXD
/* Configure USART TXD pin */
gpio_configure_pin(PIN_USART3_TXD_IDX, PIN_USART3_TXD_FLAGS);
#endif
}
