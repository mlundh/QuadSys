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

#include "../inc/string_utils.h"

uint8_t Fc_isNumericChar( uint8_t );
void Fc_reverse( uint8_t *str, int32_t length );
/* Returns the string representation of a 32bit signed integer. 
 * The function assumes that the uint8_t pointer has enough
 * space allocated to represent the whole representation.
 */
uint8_t Fc_itoa( int32_t val, uint8_t *out_str )
{
  // if negative, need 1 byte for the sign
  int sign = val < 0? 1: 0;
    int i = 0;

  // if the number is 0 then return '0'
  if (val == 0) {
    out_str[i++] = '0';
  }
    else if (val < 0)
    {
        out_str[i++] = '-';
        val = -val;
    }
  while (val > 0) {
    out_str[i++] = '0' + val % 10;
    val /= 10;
  }
  // null terminate the string
  out_str[i] = '\0';

  // reverse order on all digits, sign is already in the right place.
  Fc_reverse(out_str + sign, i - sign );
  return i;
}

void Fc_reverse( uint8_t *str, int32_t length )
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

/* Non standard atoi function, takes a buffer and a length.
 * Buffer - uint8_t *
 * length - number of bytes to be converted
 */

int32_t Fc_atoi( uint8_t *buffer, uint8_t length )
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
    if ( Fc_isNumericChar( buffer[index] ) == 0 )
    {
      return 0;
    }
    res = res * 10 + buffer[index] - '0';
  }

  // Return result with sign
  return sign * res;
}

// A utility function to check whether x is numeric
uint8_t Fc_isNumericChar( uint8_t x )
{
  return (x >= '0' && x <= '9') ? 1 : 0;
}


uint8_t *FcString_GetStringBetween(uint8_t *StringBuffer, uint32_t stringBufferLength
    , uint8_t *buffer, uint32_t buffer_length
    , const char start, const char end)
{
  StringBuffer[0] = '\0';
  if(!StringBuffer || !buffer || !start || !end)
  {
    return NULL;
  }
  char * matchFront;
  char * matchBack;
  //Find first occurence of "/" in buffer.
  matchFront = strchr ((char *)buffer, (int)start);
  matchBack = strchr ((char *)buffer, (int)end);


  //Make sure it is a valid match, and that the match fits in the provided buffer.
  if(!matchFront
      || !matchBack
      || (matchFront > matchBack )
      || (((matchBack - matchFront +1) ) > stringBufferLength))
  {
    return NULL;
  }

  uint32_t length = (uint32_t)(matchBack - matchFront);

  // Copy the value string (everything between "matchFront" and "matchBack") to value buffer,
  // discarding the indicators. Make sure that we only copy valid data, and that the data will fit
  // in target.
  if(length > stringBufferLength || (length > (buffer_length - ((char *)buffer - matchFront))))
  {
    return NULL;
  }
  strncpy ((char *)StringBuffer, (matchFront+1), length-1);
  StringBuffer[ length - 1] = '\0';

  // Return pointer to character after match.
  return (uint8_t *)(matchBack + 1);
}

uint8_t *FcString_GetStringUntil(uint8_t *toBuffer, uint32_t toBufferLength, uint8_t *fromBuffer, uint32_t frombufferLength
    , const char* delimiters, uint32_t noDelimiters)
{
  toBuffer[0]= '\0';
  if(!toBuffer || !fromBuffer || !delimiters)
  {
    return NULL;
  }
  char * firstOccurence = NULL;

  //Find first occurence of the delimiters.
  for (uint32_t i = 0; i < noDelimiters; i++)
  {
    char * current = strchr ((char *)fromBuffer, (int)delimiters[i]);
    if(!firstOccurence)
    {
      firstOccurence = current;
    }
    else if (current && (current < firstOccurence))
    {
      firstOccurence = current;
    }
  }

  if(!firstOccurence || firstOccurence < (char *)fromBuffer || (firstOccurence > (char *)(fromBuffer + frombufferLength)))
  {
    return NULL;
  }
  uint32_t length = (uint32_t)(firstOccurence - (char *)fromBuffer);
  // target has to be long enough for text + null terminaing.
  if((( length + 1 ) > toBufferLength))
  {
    return NULL;
  }
  //Copy length number of bytes to target, omitting the matched char.
  strncpy ((char *)toBuffer, (char *)fromBuffer, length);
  toBuffer[ length ] = '\0';
  // Return pointer to character after match.
  return (uint8_t *)(firstOccurence);
}

uint8_t *FcString_GetValueString(uint8_t *valueBuffer, uint32_t valueBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
  return FcString_GetStringBetween(valueBuffer, valueBufferLength, moduleBuffer, moduleBufferLength, '[', ']');
}

uint8_t *FcString_GetValueTypeString(uint8_t *valueTypeBuffer, uint32_t valueTypeBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
  return FcString_GetStringBetween(valueTypeBuffer, valueTypeBufferLength, moduleBuffer, moduleBufferLength, '<', '>');
}

uint8_t *FcString_GetModuleString(uint8_t *moduleBuffer, uint32_t moduleBufferLength, uint8_t *pathBuffer, uint32_t PathBufferLength)
{
  const char delimiters[]={"/\0"};
  uint8_t * after_string = FcString_GetStringUntil(moduleBuffer, moduleBufferLength, pathBuffer, PathBufferLength, delimiters, 1);
  if(!after_string)
  {
    return NULL;
  }
  return (after_string + 1);
}

uint8_t *FcString_GetNameString(uint8_t *nameBuffer, uint32_t nameBufferLength, uint8_t *moduleBuffer, uint32_t moduleBufferLength)
{
  const char delimiters[]={'<','[','/','\0'};
  uint8_t * after_string = FcString_GetStringUntil(nameBuffer, nameBufferLength, moduleBuffer, moduleBufferLength, delimiters, 3);
  if(!after_string)
  {
    // Found no match for delimiters in module string, module string contains only name.
    uint32_t length = 0;
    // Make sure that the string in moduleBuffer can fit in nameBuffer.Do not copy more than what is left in moduleBuffer.
    uint32_t stringLength = (uint32_t)strlen((char *)moduleBuffer);
    length = ((nameBufferLength < moduleBufferLength) ? nameBufferLength : moduleBufferLength);
    if(stringLength+1 > length)
    {
      return NULL;
    }

    strncpy ((char *)nameBuffer, (char *)moduleBuffer, stringLength+1);
    return (nameBuffer + stringLength);
  }
  return (after_string);
}


