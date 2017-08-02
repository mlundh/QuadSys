/*
 * log_backend.h
 *
 * Copyright (C) 2017 Martin Lundh
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
#ifndef MODULES_LOG_INC_LOG_BACKEND_H_
#define MODULES_LOG_INC_LOG_BACKEND_H_

#include <stdint.h>
#include <stddef.h>

typedef struct logBackEnd LogBackEnd_t;

typedef struct logEntry logEntry_t;

LogBackEnd_t* LogBackend_CreateObj();

uint8_t LogBackend_Report(LogBackEnd_t* obj, logEntry_t* entry);

uint8_t LogBackend_GetLog(LogBackEnd_t* obj, logEntry_t* loggers, uint32_t size, uint32_t* nrLogs);

#endif /* MODULES_LOG_INC_LOG_BACKEND_H_ */
