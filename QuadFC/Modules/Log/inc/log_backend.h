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

/**
 * Create a LogBackend instance.
 * @return
 */
LogBackEnd_t* LogBackend_CreateObj();

/**
 * Write one log entry into the backend. This is called by the logHandler object.
 * @param obj       LogBackend object.
 * @param entry     Log entry to write to the backend.
 * @return
 */
uint8_t LogBackend_Report(LogBackEnd_t* obj, logEntry_t* entry);

/**
 *
 * @param obj           LogBackend object.
 * @param loggers       Return array of loggers.
 * @param nrElements    Numbers of elements in the return array.
 * @param nrLogs        Number of logs actually written to the return array.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t LogBackend_GetLog(LogBackEnd_t* obj, logEntry_t* loggers, uint32_t nrElements, uint32_t* nrLogs);

#endif /* MODULES_LOG_INC_LOG_BACKEND_H_ */
