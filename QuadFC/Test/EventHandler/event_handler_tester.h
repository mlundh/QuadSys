/*
 * event_handler_tester.h
 *
 * Copyright (C) 2016 Martin Lundh
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
#ifndef TEST_EVENTHANDLER_EVENT_HANDLER_TESTER_H_
#define TEST_EVENTHANDLER_EVENT_HANDLER_TESTER_H_

#include "Test/TestFW/test_framework.h"

/**
 * Get all testcases and do needed initializations.
 * @param obj    test fw object.
 */
void EventHandler_GetTCs(TestFw_t* obj);


/**
 * Test the event circular buffer. Test basic read and write.
 * @param obj     TestFW object.
 * @return        1 if test pass, 0 if fail.
 */
uint8_t EventHandler_TestCBReadWrite(TestFw_t* obj);

/**
 * Test the circular buffers wrap around.
 * @param obj     TestFW object.
 * @return        1 if test pass, 0 if fail.
 */
uint8_t EventHandler_TestCBFullEmpty(TestFw_t* obj);

/**
 * Test event system.
 *
 * The tester consists of three test tasks and one stimuli task. All
 * four tasks report back to the thread the test case is called from via a
 * special queue.
 *
 * The test does the following:
 *
 * 1. The test creates the tasks used in the test. All event subscriptions are set here.
 * 2. The event system is initialized by all tasks.
 * 3. The stimuli task sends a number of events to all subscribers.
 * 4. The receiving tasks sends notifications to the test case when they receive
 * the events.
 * 5. The test case evaluates the results.
 *
 * 6. The stimuli task sends another event. One receiver (task 3) handle this by
 * going into a "wait for event" mode.
 * 7. The stimuli task sends an events that do not match the event
 * the receiver is waiting for. These should not be handled yet.
 * 8. The stimuli task sends the event the receiver is waiting for. This is handled
 * and the receiver sends a notification back to the test case.
 * 9. The receiver then handles all the queued events and sends corresponding information
 * to the test case.
 * 10. The test case verifies that it gets the correct events in the correct order
 * and then reports pass or fail.
 *
 * @param obj     TestFW object.
 * @return        1 if test pass, 0 if fail.
 */
uint8_t EventHandler_TestEvents(TestFw_t* obj);
#endif /* TEST_EVENTHANDLER_EVENT_HANDLER_TESTER_H_ */
