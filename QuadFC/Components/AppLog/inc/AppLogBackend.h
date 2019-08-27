/*
 * AppLogBackend.h
 *
 *  Created on: Aug 12, 2019
 *      Author: mlundh
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

#ifndef COMPONENTS_APPLOG_INC_APPLOGBACKEND_H_
#define COMPONENTS_APPLOG_INC_APPLOGBACKEND_H_

#include <stdint.h>
#include <stddef.h>

typedef struct AppLogBackend AppLogBackend_t;

/**
 * Create an AppLogBackend instance.
 * @return
 */
AppLogBackend_t* AppLogBackend_CreateObj();


/**
 * Delete the backend object. Use with caution, no other cleanup (de-registering etc.) is done.
 * @return
 */
void AppLogBackend_DeleteObj(AppLogBackend_t* obj);
/**
 * Write one log entry into the backend. This is called by the ApplogHandler object.
 * @param obj       AppLogBackend object.
 * @param entry     Buffer containing the appLog entry
 * @param buffer_size Size of the appLog entry.
 * @return
 */
uint8_t AppLogBackend_Report(AppLogBackend_t* obj, uint8_t* buffer, uint32_t buffer_size);

/**
 *
 * @param obj           LogBackend object.
 * @param buffer        The buffer to populate with the appLog.
 * @param buffer_size   Length of the buffer.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t AppLogBackend_GetLog(AppLogBackend_t* obj, uint8_t* buffer, uint32_t buffer_size);


#endif /* COMPONENTS_APPLOG_INC_APPLOGBACKEND_H_ */
