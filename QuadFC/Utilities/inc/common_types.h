/*
 * common_types.h
 *
 * Copyright (C) 2014 martin
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
#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_
#include "compiler.h"

typedef enum state_names
{
  pitch = 0,
  roll = 1,
  yaw = 2,
  pitch_rate = 3,
  roll_rate = 4,
  yaw_rate = 5,
  x_pos = 6,
  y_pos = 7,
  z_pos = 8,
  x_vel = 9,
  y_vel = 10,
  z_vel = 11,
  nr_states = 12,
}state_names_t;

typedef struct state_data
{
  int32_t state_vector[12];
  uint8_t confidence[12];
}state_data_t;

typedef enum fc_error_code
{
  ok = 0,
  ec_timeout = 1,
  ec_not_allowed = 2,
}fc_error_code_t;


#endif /* COMMON_TYPES_H_ */
