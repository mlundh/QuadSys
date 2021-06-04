/*
* QuadFC_Pwm.h
*
* Copyright (C) 2021  Martin Lundh
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
#ifndef __QUADFC_PWM_H__
#define __QUADFC_PWM_H__

#include <stdint.h>

typedef enum{
    pwm_50,
    pwm_OneShot125,
    pwm_OneShot42,
} Pwm_Mode_t;

/**
 * @brief Initialize the PWM module to a standard mode defined in the Pwm_Modue_t enum.
 * 
 * @param pin           PWM pin, use defines in the BoardConfig.h file. 
 * @param mode          Mode defined in the above enum.
 * @return uint8_t      1 if success, 0 if the configuration is not possible.
 */
uint8_t PWM_Initialize(uint8_t pin, Pwm_Mode_t mode);

/**
 * @brief Initialize the PWM module to a custom mode.
 * 
 * @param pin               PWM pin, use defines in the BoardConfig.h file.
 * @param bitsResolution    Number of bits of resolution needed.
 * @param frequencyHz       Base frequency of the PWM signal. 
 * @return uint8_t 
 */
uint8_t PWM_InitializeCustom(uint8_t pin, uint8_t bitsResolution, uint32_t frequencyHz);

/**
 * @brief Set or update the duty cycle of the pin.
 * 
 * @param pin       A pin that has already been initialized.
 * @param duty      Duty cycle in percent. 16 bits are used
 * @return uint8_t 
 */
uint8_t PWM_SetDutyCycle(uint8_t pin, uint32_t duty);

/**
 * @brief Enable the output of the pin.
 * 
 * @param pin       A pin that has already been initialized.
 * @return uint8_t  1 if success 0 if fail.
 */
uint8_t PWM_Enable(uint8_t pin);

/**
 * @brief Disable the output of the pin. 
 * 
 * @param pin       A pin that has already been initialized.
 * @return uint8_t  1 if success 0 if fail.
 */
uint8_t PWM_Disable(uint8_t pin);


#endif // __QUADFC_PWM_H__
