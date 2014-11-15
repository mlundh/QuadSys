/*
 * slip_utils.c
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

#include "slip_utils.h"

uint8_t Slip_Enocode(uint8_t* frame_buffer, uint8_t* serialized_data, uint8_t length)
{
  frame_buffer[0] = frame_boundary_octet;
  int k;
  int i;
  int nrEscapeOctets = 0;
  for ( i = 0, k = 1; i < (length); i++, k++ )
  {
    if ( serialized_data[i] == frame_boundary_octet )
    {
      frame_buffer[k++] = control_escape_octet;
      frame_buffer[k] = frame_boundary_octet_replacement;
      nrEscapeOctets++;
    }
    else if ( serialized_data[i] == control_escape_octet )
    {
      frame_buffer[k++] = control_escape_octet;
      frame_buffer[k] = control_escape_octet_replacement;
      nrEscapeOctets++;
    }
    else
    {
      frame_buffer[k] = serialized_data[i];
    }
  }
  frame_buffer[k] = frame_boundary_octet;
  return ++k;
}



uint8_t Slip_Decode(uint8_t* frame_buffer, uint8_t* serialized_data, uint8_t length)
{
  int k;
  int i;
  if(frame_buffer[0] != frame_boundary_octet)
  {
    return 0; // Todo, error!
  }
  for ( i = 0, k = 1; k < (length - 1); i++, k++ )
  {
    if ( frame_buffer[k] == control_escape_octet )
    {
      k++;
      if ( frame_buffer[k] == control_escape_octet_replacement )
      {
        serialized_data[i] = control_escape_octet;
      }
      else if ( frame_buffer[k] == frame_boundary_octet_replacement )
      {
        serialized_data[i] = frame_boundary_octet;
      }
      else
      {
        return 0; //Todo, error!
      }
    }
    else
    {
      serialized_data[i] = frame_buffer[k];
    }
  }
  return ++i;
}
