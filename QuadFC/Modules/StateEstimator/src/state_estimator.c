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
#include "StateEstimator/inc/state_estimator.h"
#include "StateEstimator/inc/imu_signal_processing.h"
#include "QuadFC/QuadFC_IMU.h"

struct StateEst
{
  Imu_t * imu;
  estimation_types_t est_type;
};

StateEst_t *StateEst_Create()
{
  StateEst_t * stateEstObj = pvPortMalloc(sizeof(StateEst_t));
  if(!stateEstObj)
  {
    return NULL;
  }
  stateEstObj->imu = Imu_Create();
  stateEstObj->est_type = type_not_availible;

  if(!stateEstObj->imu)
  {
    return NULL;
  }
  return stateEstObj;
}


uint8_t StateEst_init(StateEst_t *obj, estimation_types_t type)
{
  obj->est_type = type;
  uint8_t result = Imu_Init(obj->imu);
  return result;
}

uint8_t StateEst_getState(StateEst_t *obj, state_data_t *state_vector)
{
  Imu_GetData(obj->imu);
  switch (obj->est_type)
  {
  case raw_data_rate:
    get_rate_gyro( state_vector, &obj->imu->ImuData );
    break;
  default:
    return 0;
  }
  return 1;
}

uint8_t StateEst_SetEstType(StateEst_t *obj, estimation_types_t est)
{
  obj->est_type = est;
  return 1;
}

estimation_types_t StateEst_GetEstType(StateEst_t *obj)
{
  return obj->est_type;
}
