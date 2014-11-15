/*
 * slip_utils.h
 *
 * Copyright (C) 2015 martin
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
#ifndef SLIP_UTILS_H_
#define SLIP_UTILS_H_

#include "stdint.h"

enum Slip_Control_Octets
{
  frame_boundary_octet = 0x7E,
  frame_boundary_octet_replacement = 0x5E,
  control_escape_octet = 0x7D,
  control_escape_octet_replacement = 0x5D
};


uint8_t Slip_Enocode(uint8_t* frame_buffer, uint8_t* serialized_data, uint8_t length);

uint8_t Slip_Decode(uint8_t* frame_buffer, uint8_t* serialized_data, uint8_t length);



#endif /* SLIP_UTILS_H_ */
