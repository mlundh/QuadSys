/*
 * serialization.c
 *
 * Copyright (C) 2017 Martin Lundh
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
#include "Utilities/inc/serialization.h"
uint32_t serialize_int32(uint8_t *buffer, uint32_t size, int32_t value)
{
  if(!buffer || size < 4)
  {
    return 0;
  }
  buffer[0]  = (uint8_t)((value >> 24)   & 0xFF);
  buffer[1]  = (uint8_t)((value >> 16)   & 0xFF);
  buffer[2]  = (uint8_t)((value >> 8)    & 0xFF);
  buffer[3]  = (uint8_t)((value)         & 0xFF);
  return 4;
}

uint32_t deserialize_int32(uint8_t *buffer, uint32_t size, int32_t* value)
{
  if(!buffer || size < 4)
  {
    return 0;
  }
  *value = (((int32_t)buffer[0]) << 24)+
           (((int32_t)buffer[1]) << 16) +
           (((int32_t)buffer[2]) << 8) +
            ((int32_t)buffer[3]);
           ;
  return 4;
}
