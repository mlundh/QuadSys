/*
 * QuadFC_IMU.h
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
#ifndef HAL_INC_QUADFC_IMU_H_
#define HAL_INC_QUADFC_IMU_H_

/**
 * @file QuadFC_IMU.h
 *
 * Interface responsible for handling IMUs in the system.
 */

#include <stdint.h>
#include "Messages/inc/common_types.h"
#include "Parameters/inc/parameters.h"

typedef struct ImuInternals ImuInternals_t;


typedef struct Imu
{
  ImuData_t ImuOffset;
  ImuData_t ImuData;  // Copter coordinate system.
  ImuData_t tempData; // IMU coordinate system
  ImuOrientation_t Orient;
  ImuInternals_t *internals;
}Imu_t;

/**
 * Create the IMU unit. This function should be called before the
 * scheduler is started.
 * @param param a root parameter for all Imu related parameters.
 * @return            A newly created IMU object. */
Imu_t * Imu_Create(param_obj_t * param);

/**
 * Initialize the imu object. This function should be called after the
 * scheduler is started.
 * @param obj         Current IMU object.
 * @return            0 if fail, 1 otherwise.
 */
uint8_t Imu_Init(Imu_t *obj);

/**
 * Get the scaled (but unfiltered) data from the imu. The state data
 * contains information of confidence, 0 confidence means not availible,
 * higher confidence means ok reading.
 * @param obj           Current IMU object.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t Imu_GetData(Imu_t *obj);

#endif /* HAL_INC_QUADFC_IMU_H_ */
