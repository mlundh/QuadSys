/*
 * QuadFC_IMU.c
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

/**
 * @file Generic implementation of the QuadFC_IMU interface.
 */

#include "stdint.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "common_types.h"
#include "parameters.h"
#include "QuadFC_IMU.h"
#include "QuadFC_IMUInternal.h"


#define SIGN(x) ((x > 0) - (x < 0))

Imu_t * Imu_Create()
{
  Imu_t *tmp = Imu_CreateInternal();
  SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

  tmp->Orient.x_sign = 1;
  tmp->Orient.y_sign = 1;
  tmp->Orient.z_sign = 1;

  param_obj_t * ImuOriRoot = Param_CreateObj(10, NoType, readOnly, NULL, "IMU_Ori", Param_GetRoot(), NULL);
  Param_CreateObj(0, int8_variable_type, readWrite,
      &tmp->Orient.x_sign, "x_sign", ImuOriRoot, xMutex);
  Param_CreateObj(0, int8_variable_type, readWrite,
      &tmp->Orient.y_sign, "y_sign", ImuOriRoot, xMutex);
  Param_CreateObj(0, int8_variable_type, readWrite,
      &tmp->Orient.z_sign, "z_sign", ImuOriRoot, xMutex);
  return tmp;
}

uint8_t Imu_Init(Imu_t *obj)
{
  uint8_t result = Imu_InitInternal(obj);
  return result;
}

uint8_t Imu_GetData(Imu_t *obj)
{
  if(!obj || !Imu_GetDataInternal(obj))
  {
    return 0;
  }
  // Get sign of orientation parameter. Uses branch-less operations to extract sign.
  obj->ImuData.imu_data[accl_x] = SIGN(obj->Orient.x_sign) * obj->tempData.imu_data[accl_x];
  obj->ImuData.imu_data[accl_y] = SIGN(obj->Orient.y_sign) * obj->tempData.imu_data[accl_y];
  obj->ImuData.imu_data[accl_z] = SIGN(obj->Orient.z_sign) * obj->tempData.imu_data[accl_z];
  obj->ImuData.imu_data[gyro_x] = SIGN(obj->Orient.x_sign) * obj->tempData.imu_data[gyro_x];
  obj->ImuData.imu_data[gyro_y] = SIGN(obj->Orient.y_sign) * obj->tempData.imu_data[gyro_y];
  obj->ImuData.imu_data[gyro_z] = SIGN(obj->Orient.z_sign) * obj->tempData.imu_data[gyro_z];
  return 1;

}


