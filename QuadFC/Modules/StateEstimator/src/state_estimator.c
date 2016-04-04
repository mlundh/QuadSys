/*
 * state_estimator.c
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

#include <stddef.h>
#include "FreeRTOS.h"

#include "StateEstimator/inc/state_estimator.h"
#include "FlightController/inc/control_mode_handler.h"

#include "../inc/signal_processing.h"
#include "QuadFC/QuadFC_IMU.h"

struct StateEst
{
  Imu_t * imu;
  CtrlModeHandler_t* CtrlModeHandler;
};

StateEst_t *StateEst_Create(CtrlModeHandler_t* CtrlModeHandler)
{
  StateEst_t * stateEstObj = pvPortMalloc(sizeof(StateEst_t));
  if(!stateEstObj)
  {
    return NULL;
  }
  stateEstObj->CtrlModeHandler = CtrlModeHandler;
  stateEstObj->imu = Imu_Create();

  if(!stateEstObj->imu)
  {
    return NULL;
  }
  return stateEstObj;
}


uint8_t StateEst_init(StateEst_t *obj)
{
  uint8_t result = Imu_Init(obj->imu);
  return result;
}

uint8_t StateEst_getState(StateEst_t *obj, state_data_t *state_vector)
{
  if(!Imu_GetData(obj->imu))
  {
    return 0;
  }
  state_vector->state_valid_bf = 0;
  switch (Ctrl_GetCurrentMode(obj->CtrlModeHandler))
  {
  case Control_mode_rate:
    Signal_getRateGyro( state_vector, &obj->imu->ImuData );
    break;
  case Control_mode_attitude:
  {
    // Variables to hold intermediate angles based on only imu data.
    state_data_t gyroStateTmp = {{0}};
    state_data_t accStateTmp = {{0}};
    //Get the sample time from the main control loop timeing.
    control_time_t sampleTime;
    sampleTime.value = CTRL_TIME_FP;

    Signal_getEulerAnglesGyro(&gyroStateTmp, &obj->imu->ImuData, &sampleTime);
    Signal_getEulerAnglesAccel(&accStateTmp, &obj->imu->ImuData);
    // Do the complementary filtering. This takes the high-frequency behavior from
    // the gyro, and the low frequency behavior from the accelerometer.
    Signal_complemetaryFilter(&accStateTmp, &gyroStateTmp, state_vector);

    // The complementary filter only estimates the pitch and roll angles, for yaw
    // we must use the rate, so get that too.
    Signal_getRateGyro( state_vector, &obj->imu->ImuData );
  }
    break;
  case Control_mode_not_available:
    /*----------------------------Mode not available------------
     * Something is very wrong, we have an unknown state.
     */
    break;
  default:
    return 0;
  }
  return 1;
}

