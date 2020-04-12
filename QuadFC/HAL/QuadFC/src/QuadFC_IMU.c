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
 *
 * The rotation matrix is denoted as follows:
 *
 * +-          -+
 * |0_0 0_1 0_2 |
 * |1_0 1_1 1_2 |
 * |2_0 2_1 2_2 |
 * +-          -+
 *
 *
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "Messages/inc/common_types.h"
#include "QuadFC/QuadFC_IMU.h"
#include "QuadFC/src/QuadFC_IMUInternal.h"
#include "Utilities/inc/my_math.h"


#define SIGN(x) ((x > 0) - (x < 0))

Imu_t * Imu_Create(param_obj_t * param)
{
  Imu_t *obj = Imu_CreateInternal(param);

  ImuOrientation_t tmp = {0};
  obj->Orient = tmp;
  obj->Orient.r_0_0 = INT_TO_FIXED(1,FP_16_16_SHIFT);
  obj->Orient.r_1_1 = INT_TO_FIXED(1,FP_16_16_SHIFT);
  obj->Orient.r_2_2 = INT_TO_FIXED(1,FP_16_16_SHIFT);
  obj->Orient.acc_sign = 1;
  obj->Orient.gyro_sign = 1;


  param_obj_t * ImuOriRoot = Param_CreateObj(12, variable_type_NoType, readOnly, NULL, "IMU_Rot", param );
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_0_0, "r_0_0", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_0_1, "r_0_1", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_0_2, "r_0_2", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_1_0, "r_1_0", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_1_1, "r_1_1", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_1_2, "r_1_2", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_2_0, "r_2_0", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_2_1, "r_2_1", ImuOriRoot);
  Param_CreateObj(0, variable_type_fp_16_16, readWrite,
      &obj->Orient.r_2_2, "r_2_2", ImuOriRoot);

  Param_CreateObj(0, variable_type_int8, readWrite,
      &obj->Orient.acc_sign, "accSign", ImuOriRoot);
  Param_CreateObj(0, variable_type_int8, readWrite,
      &obj->Orient.gyro_sign, "gyroSign", ImuOriRoot);

  param_obj_t * ImuCurrent = Param_CreateObj(10, variable_type_NoType, readOnly, NULL, "IMU_Current", param);

  Param_CreateObj(0, variable_type_fp_16_16, readOnly,
      &obj->ImuData.imu_data[accl_x], "accl_x", ImuCurrent);
  Param_CreateObj(0, variable_type_fp_16_16, readOnly,
      &obj->ImuData.imu_data[accl_y], "accl_y", ImuCurrent);
  Param_CreateObj(0, variable_type_fp_16_16, readOnly,
      &obj->ImuData.imu_data[accl_z], "accl_z", ImuCurrent);

  Param_CreateObj(0, variable_type_fp_16_16, readOnly,
      &obj->ImuData.imu_data[gyro_x], "gyro_x", ImuCurrent);
  Param_CreateObj(0, variable_type_fp_16_16, readOnly,
      &obj->ImuData.imu_data[gyro_y], "gyro_y", ImuCurrent);
  Param_CreateObj(0, variable_type_fp_16_16, readOnly,
      &obj->ImuData.imu_data[gyro_z], "gyro_z", ImuCurrent);

  return obj;
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

  obj->ImuData.imu_data[accl_x] = my_mult(obj->tempData.imu_data[accl_x], obj->Orient.r_0_0, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[accl_y], obj->Orient.r_0_1, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[accl_z], obj->Orient.r_0_2, FP_16_16_SHIFT);

  obj->ImuData.imu_data[accl_y] = my_mult(obj->tempData.imu_data[accl_x], obj->Orient.r_1_0, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[accl_y], obj->Orient.r_1_1, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[accl_z], obj->Orient.r_1_2, FP_16_16_SHIFT);

  obj->ImuData.imu_data[accl_z] = my_mult(obj->tempData.imu_data[accl_x], obj->Orient.r_2_0, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[accl_y], obj->Orient.r_2_1, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[accl_z], obj->Orient.r_2_2, FP_16_16_SHIFT);

  obj->ImuData.imu_data[accl_x] *= SIGN(obj->Orient.acc_sign);
  obj->ImuData.imu_data[accl_y] *= SIGN(obj->Orient.acc_sign);
  obj->ImuData.imu_data[accl_z] *= SIGN(obj->Orient.acc_sign);

  obj->ImuData.imu_data[gyro_x] = my_mult(obj->tempData.imu_data[gyro_x], obj->Orient.r_0_0, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[gyro_y], obj->Orient.r_0_1, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[gyro_z], obj->Orient.r_0_2, FP_16_16_SHIFT);

  obj->ImuData.imu_data[gyro_y] = my_mult(obj->tempData.imu_data[gyro_x], obj->Orient.r_1_0, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[gyro_y], obj->Orient.r_1_1, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[gyro_z], obj->Orient.r_1_2, FP_16_16_SHIFT);

  obj->ImuData.imu_data[gyro_z] = my_mult(obj->tempData.imu_data[gyro_x], obj->Orient.r_2_0, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[gyro_y], obj->Orient.r_2_1, FP_16_16_SHIFT) +
                                  my_mult(obj->tempData.imu_data[gyro_z], obj->Orient.r_2_2, FP_16_16_SHIFT);

  obj->ImuData.imu_data[gyro_x] *= SIGN(obj->Orient.gyro_sign);
  obj->ImuData.imu_data[gyro_y] *= SIGN(obj->Orient.gyro_sign);
  obj->ImuData.imu_data[gyro_z] *= SIGN(obj->Orient.gyro_sign);
  return 1;

}


