/*
 * AppLogHandler.h
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

#ifndef COMPONENTS_APPLOG_INC_APPLOGHANDLER_H_
#define COMPONENTS_APPLOG_INC_APPLOGHANDLER_H_

#include "EventHandler/inc/event_handler.h"

typedef struct AppLogHandler AppLogHandler_t;

/**
 * Create the AppLogHandler. The handler will register the handler function for
 * AppLog messages, and write the log entry to the backend. The backend is responsible
 * for writing to either memory, or serial port etc.
 * @param evHandler		Event handler.
 * @return
 */
AppLogHandler_t* AppLogHandler_Create(eventHandler_t* evHandler);


#endif /* COMPONENTS_APPLOG_INC_APPLOGHANDLER_H_ */
