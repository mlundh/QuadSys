/*
 * Satellite_SetpointHandler.h
 *
 * Copyright (C) 2020 Martin Lundh
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

#ifndef _SATELLITE_SETPOINTHANDLER_H_
#define _SATELLITE_SETPOINTHANDLER_H_

#include "SpectrumSatellite/inc/satellite_receiver_public.h"

#define SATELLITE_CH_CENTER (1022)

struct spectrumSpHandler
{
  eventHandler_t*       evHandler;
  state_data_t*         currentSetpoint;
  int32_t               multiplier;
  int32_t               throMult;
  FlightMode_t          current_flight_mode_state;
  CtrlMode_t            current_control_mode;
  TickType_t            timeAtLastNewSp;
};

typedef struct spectrumSpHandler spectrumSpHandler_t;

/**
 * Map receiver channels to setpoint.
 *
 * Scaling of state for rate is 1pi rad/s = 1<<16. Max (default) rate from receiver is 2pi rad/s.
 * Scaling of state for angle is 1pi rad = 1<<16. Max (default) rate from receiver is pi rad.
 *
 * @param obj               The instance object.
 * @param reciever_data     The valid and complete frames.
 * @param setpoint          Output, setpoint.
 */
void SatSpHandler_MapToSetpoint(spectrumSpHandler_t* obj, spektrum_data_t *reciever_data, state_data_t *setpoint);

/**
 * @brief Create a spectrum satellite handler object. This object will handle one or multiple spectrum satellite
 * instnaces. The module will communicate over the event system.
 * 
 * @param evHandler                 Event handler to be used.
 * @param param                     Root parameter, this module adds three paramters.
 * @return spectrumSpHandler_t*     Instance of the handler. Null if failed.
 */
spectrumSpHandler_t* SatSpHandler_CreateObj(eventHandler_t* evHandler, param_obj_t * param);

/**
 * @brief Delete the object.
 * 
 * @param obj       Object to delete.
 */
void SatSpHandler_DeleteObj(spectrumSpHandler_t* obj);



#endif // _SATELLITE_SETPOINTHANDLER_H_
