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
#ifndef HAL_INC_QUADFC_IMUINTERNAL_H_
#define HAL_INC_QUADFC_IMUINTERNAL_H_

/**
 * @file QuadFC_IMUInternal.h
 *
 * Interface required to be implemented by any IMU in the system. This
 * internal interface is used by the external interface QuadFC_IMU.
 */


#include <stdint.h>
#include "Messages/inc/common_types.h"
#include "Parameters/inc/parameters.h"
/**
 * Create the IMU unit. This function should be called before the
 * scheduler is started.
 * @param param a root parameter to use if the internal data should be configurable.
 * @return            A newly created IMU object. */
Imu_t * Imu_CreateInternal(param_obj_t * param);

/**
 * Initialize the imu object. This function should be called after the
 * scheduler is started.
 * @param obj         Current IMU object.
 * @return            0 if fail, 1 otherwise.
 */
uint8_t Imu_InitInternal(Imu_t *obj);

/**
 * Internal function that adds sensor orientation information to the object.
 * Any implementation has to implement this functionallity.
 * @param obj           Current IMU object.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t Imu_GetDataInternal(Imu_t *obj);

#endif /* HAL_INC_QUADFC_IMUINTERNAL_H_ */
