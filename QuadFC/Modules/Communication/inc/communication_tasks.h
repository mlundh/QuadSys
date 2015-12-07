/*
 * communication_tasks.h
 *
 * Copyright (C) 2015 Martin Lundh
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



#ifndef COMMUNICATION_TASKS_H_
#define COMMUNICATION_TASKS_H_

#include "crc.h"
#include "stdint.h"
#include "slip_packet.h"
#include "quad_serial_packet.h"

/**
 * Create the communication tasks. These tasks are responsible for
 * communication with external components such as QuadGS. They are
 * also responsible for setting parameters.
 */
void Com_CreateTasks( void );

/**
 * Interface function for sending a QSP to the external component (QuadGS).
 * @param packet  QSP to send.
 * @return        1 if sucessful, 0 otherwise.
 */
uint8_t Com_SendQSP( QSP_t *packet );


#endif /* COMMUNICATION-TASKS-H- */
