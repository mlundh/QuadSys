/*
 * range_meter.h  .gyro_x = 0,
    .
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
#ifndef RANGE_METER_H_
#define RANGE_METER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "compiler.h"

#define RANGE_METER_MESSAGE_LENGTH (6)
#define BAUD_RANGE_METER (9600)
#define RANGE_USART USART3

/*New range data available queue*/
#define RANGER_QUEUE_LENGTH         (1)
#define RANGER_QUEUE_ITEM_SIZE      (sizeof(int32_t))
 QueueHandle_t xQueue_ranger;

void create_range_meter_task( void );
void range_meter_task( void *pvParameters );

#endif /* RANGE_METER_H_ */
