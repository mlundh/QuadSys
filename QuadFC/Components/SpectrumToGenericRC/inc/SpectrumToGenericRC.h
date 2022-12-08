/*
* SpectrumToGenericRC.h
*
* Copyright (C) 2022  Martin Lundh
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
#ifndef __SPECTRUMTOGENERICRC_H__
#define __SPECTRUMTOGENERICRC_H__

#include "SpectrumSatellite/inc/satellite_receiver_public.h"

#define RC_CH_CENTER (1022)

enum channels{
    channel0,
    channel1,
    channel2,
    channel3,
    channel4,
    channel5,
    channel6,
    channel7,
    channel8,
    channel9,
    channel10,
    channel11,
};



struct spectrumGenericHandler
{
  eventHandler_t*       evHandler;
  genericRC_t*          currentSignal;
  TickType_t            timeAtLastSignal;
};

typedef struct spectrumGenericHandler spectrumGenericHandler_t;

/**
 * Map receiver channels to generic RC.
 *
 * Scaling of state for rate is 1pi rad/s = 1<<16. Max (default) rate from receiver is 2pi rad/s.
 * Scaling of state for angle is 1pi rad = 1<<16. Max (default) rate from receiver is pi rad.
 *
 * @param obj               The instance object.
 * @param reciever_data     The valid and complete frames.
 * @param setpoint          Output, setpoint.
 */
void SatGenericHandler_MapToGeneric(spectrumGenericHandler_t* obj, spektrum_data_t *reciever_data, genericRC_t *setpoint);

/**
 * @brief Create a spectrum satellite generic handler object. This object will handle one or multiple spectrum satellite
 * instnaces. The module will communicate over the event system.
 * 
 * @param evHandler                 Event handler to be used.
 * @return spectrumSpHandler_t*     Instance of the handler. Null if failed.
 */
spectrumGenericHandler_t* SatGenericHandler_CreateObj(eventHandler_t* evHandler);

/**
 * @brief Delete the object.
 * 
 * @param obj       Object to delete.
 */
void SatGenericHandler_DeleteObj(spectrumGenericHandler_t* obj);


#endif // __SPECTRUMTOGENERICRC_H__
