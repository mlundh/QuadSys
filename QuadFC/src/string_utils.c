/*
 * prepare_output.c
 *
 * Copyright (C) 2014  Martin Lundh
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

#include "string_utils.h"
uint8_t isNumericChar( uint8_t );
void reverse( uint8_t *str, int32_t length );
/* Returns the string representation of a 32bit signed integer. 
 * The function assumes that the uint8_t pointer has enough
 * space allocated to represent the whole representation.
 */
uint8_t my_itoa( int32_t val, uint8_t **out_str )
{
  // if negative, need 1 byte for the sign
  int sign = val < 0 ? 1 : 0;
  uint8_t i = 0;

  //if the number is 0 then return '0'
  if ( val == 0 )
  {
    *(*out_str + i++) = '0';
  }
  else if ( val < 0 )
  {
    *(*out_str + i++) = '-';
    val = -val;
  }
  while ( val > 0 )
  {
    *(*out_str + i++) = '0' + val % 10;
    val /= 10;
  }

  // reverse order on all digits, sign is already in the right place.
  reverse( *out_str + sign, i - sign );
  *out_str = (*out_str + i);
  return i;
}

void reverse( uint8_t *str, int32_t length )
{
  int i = 0, j = length - 1;
  uint8_t tmp;
  while ( i < j )
  {
    tmp = str[i];
    str[i] = str[j];
    str[j] = tmp;
    i++;
    j--;
  }
}

uint8_t end_string_print( uint8_t **str )
{
  uint8_t i = 0;
  *(*str + i++) = '\r';
  *(*str + i++) = '\n';
  *(*str + i++) = '\0';
  *str = *str + i;
  return i;
}

/* Non standard atoi function, takes a buffer and a length.
 * Buffer - uint8_t *
 * length - number of bytes to be converted
 */

int32_t my_atoi( uint8_t *buffer, uint8_t length )
{
  if ( buffer == NULL )
  {
    return 0;
  }

  int32_t res = 0;
  int sign = 1;  // Sign is positive if not stated otherwise
  int index = 0;  // First digit

  // Check and update the sign if nessesary.
  if ( buffer[0] == '-' )
  {
    sign = -1;
    index++;  // Update index of first digit
  }

  for ( ; index < length; ++index )
  {
    if ( isNumericChar( buffer[index] ) == 0 )
      return 0;
    //TODO error!
    res = res * 10 + buffer[index] - '0';
  }

  // Return result with sign
  return sign * res;
}

// A utility function to check whether x is numeric
uint8_t isNumericChar( uint8_t x )
{
  return (x >= '0' && x <= '9') ? 1 : 0;
}
