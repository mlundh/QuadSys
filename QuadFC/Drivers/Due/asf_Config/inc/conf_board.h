/**
 * \file
 *
 * \brief Board configuration.
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

#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED

/** Configure PWM pins used to control motors */
#define CONF_BOARD_PWM_PIN_9
#define CONF_BOARD_PWM_PIN_8
#define CONF_BOARD_PWM_PIN_7
#define CONF_BOARD_PWM_PIN_6
#define CONF_BOARD_PWM_PIN_34
#define CONF_BOARD_PWM_PIN_36
#define CONF_BOARD_PWM_PIN_38
#define CONF_BOARD_PWM_PIN_40

#define CONF_BOARD_SPI0

#define CONF_BOARD_SPI0_NPCS0

#define CONF_BOARD_TWI0

#define CONF_BOARD_TWI1

/* Configure USART0 RXD pin */
#define CONF_BOARD_USART0_RXD

/* Configure USART0 TXD pin */
#define CONF_BOARD_USART0_TXD

/* Configure USART1 RXD pin */
#define CONF_BOARD_USART1_RXD

/* Configure USART1 TXD pin */
#define CONF_BOARD_USART1_TXD

/* Configure USART3 RXD pin */
#define CONF_BOARD_USART3_RXD

/* Configure USART3 TXD pin */
#define CONF_BOARD_USART3_TXD


#endif /* CONF_BOARD_H_INCLUDED */
