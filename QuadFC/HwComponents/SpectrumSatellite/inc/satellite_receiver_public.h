/*
 * satellite_receiver_public.h
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
#ifndef PORTLAYER_COMMUNICATION_INC_SATELLITE_RECEIVER_PUBLIC_H_
#define PORTLAYER_COMMUNICATION_INC_SATELLITE_RECEIVER_PUBLIC_H_
#include "SetpointHandler/inc/setpoint_handler.h"
#include "Messages/inc/common_types.h"

/**
 * Public interface for the satellite receiver task.
 */
void Satellite_CreateReceiverTask( QueueHandle_t eventHandler);

#endif /* PORTLAYER_COMMUNICATION_INC_SATELLITE_RECEIVER_PUBLIC_H_ */
