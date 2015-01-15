/*
 * globals.h
 *
 * Copyright (C) 2014 martin
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
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "freertos_twi_master.h"
#include "queue.h"

extern SemaphoreHandle_t x_param_mutex;
extern SemaphoreHandle_t x_log_mutex;
extern SemaphoreHandle_t twi_0_notification_semaphore;
extern SemaphoreHandle_t twi_1_notification_semaphore;
extern freertos_twi_if twi_0;
extern freertos_twi_if twi_1;

#define mainDONT_BLOCK                          (0)
/*
 * Fixed point scaling factor.
 */
#define SHIFT_EXP 6

#endif /* GLOBALS_H_ */
