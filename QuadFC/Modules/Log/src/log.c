/*
 * log.c
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

#include "log.h"
#include "string.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "string_utils.h"

log_obj_t *log_objects[MAX_LOG_GROUPS];
uint8_t last_log_obj = 0;

log_obj_t *Log_CreateLogObj(uint8_t num_variables, Log_variable_type_t type, const char *group_name, Log_LogFunction_t function)
{
  // Do some input checks.
  if(num_variables > MAX_LOG_VAR_PER_GROUP || function == NULL)
  {
    return NULL;
  }
  // Some more checking and type identification.
  int size = 0;
  switch ( type )
  {
  case uint8_variable_type:
  case int8_variable_type:
    size = (sizeof( uint8_t ) * num_variables);
    break;
  case uint16_variable_type:
  case int16_variable_type:
    size = (sizeof( uint16_t ) * num_variables);
    break;
  case uint32_variable_type:
  case int32_variable_type:
    size = (sizeof( uint32_t ) * num_variables);
    break;
  default:
    return NULL;
  }

  log_obj_t *log_obj = pvPortMalloc( sizeof(log_obj_t) );

  log_obj->num_variables = num_variables;
  log_obj->type = type;
  log_obj->group_name = pvPortMalloc( sizeof( unsigned char ) * MAX_LOG_NAME_LENGTH );
  log_obj->var_name = pvPortMalloc(sizeof( unsigned char *) * num_variables);
  log_obj->serialized_data = pvPortMalloc(sizeof( unsigned char *) * num_variables);
  log_obj->function = function;
  log_obj->logging_active = 0;
  for(int i = 0; i < num_variables; i++)
  {
    //alloc and set mem to 0;
    log_obj->var_name[i] = pvPortMalloc( (sizeof( unsigned char )) * MAX_LOG_NAME_LENGTH );
    memset(( char * ) log_obj->var_name[i], 0, ( unsigned short ) MAX_LOG_NAME_LENGTH);
  }
  strncpy( ( char * ) log_obj->group_name, ( const char * ) group_name, ( unsigned short ) MAX_LOG_NAME_LENGTH );

  for(int i = 0; i < num_variables; i++)
  {
    //alloc and set mem to 0;
    log_obj->serialized_data[i] = pvPortMalloc( size );
    memset(( char * ) log_obj->serialized_data[i], 0, size);
  }
  return log_obj;
}


void Log_SetLVaiableName(log_obj_t *log_obj, const char *var_name, uint8_t var_index)
{
  if(var_index >= log_obj->num_variables)
  {
    return;
  }
  strncpy( ( char * ) log_obj->var_name[var_index], ( const char * ) var_name, ( unsigned short ) MAX_LOG_NAME_LENGTH );
}

void Log_register_log_group(log_obj_t *log_obj)
{
  if(!log_obj && (last_log_obj < MAX_LOG_GROUPS) )
  {
    return;
  }
  log_objects[last_log_obj] = log_obj;
  last_log_obj++;

}

uint8_t * Log_SerializeInt32(uint8_t *buffer, int32_t value)
{

    buffer[0] = value >> 24;
    buffer[1] = value >> 16;
    buffer[2] = value >> 8;
    buffer[3] = value;
    return buffer + 4;
}

uint8_t * Log_SerializeInt16(uint8_t *buffer, int16_t value)
{
    buffer[0] = value >> 8;
    buffer[1] = value;
    return buffer + 2;
}

uint8_t * Log_SerializeInt8(uint8_t *buffer, int8_t value)
{
    buffer[0] = value;
    return buffer + 1;
}
int32_t Log_DeserializeInt32(uint8_t *buffer)
{
  int32_t value = 0;

  value |= buffer[0] << 24;
  value |= buffer[1] << 16;
  value |= buffer[2] << 8;
  value |= buffer[3] ;
    return value;
}

int16_t Log_DeserializeInt16(uint8_t *buffer)
{
  int16_t value = 0;
  value |= buffer[0] << 8;
  value |= buffer[1] ;
    return value;
}

uint8_t Log_DeserializeInt8(uint8_t *buffer)
{
  uint8_t value = 0;
    value = buffer[0];
    return value;
}



/**
 * internal section
 */

void Log_DoLogging()
{
  for(int i = 0; i < last_log_obj; i++)
  {
    if(log_objects[i]->logging_active != 0)
    {
      log_objects[i]->function(log_objects[i]);
    }
  }
}



/**
 * Copy all group names to buffer.
 */
void Log_GetGroups(uint8_t *buffer, uint8_t buffer_length)
{
  // make sure buffer is empty.
  buffer[0] = '\0';
  uint8_t buf_index = 0;
  for(int i = 0; i < last_log_obj; i++)
  {
    //ensure that the name will fit in current buffer.
    if((buf_index + MAX_LOG_NAME_LENGTH + CTRL_BYTES + 1) > buffer_length)
    {
      return;
    }
    //append group specifier "\g".
    strncat( (char *) buffer, (const char *) "\\g", (unsigned short) 2);
    //append group name and update buf_index.
    strncat( (char *) buffer , (const char *) log_objects[i]->group_name, (unsigned short) MAX_LOG_NAME_LENGTH);
    // append variabletype specifier "/t" and uppdate buf_index.,
    strncat( (char *) buffer, (const char *) "\\t", (unsigned short) 2);
    uint8_t pTemp[5];
    my_itoa((int32_t)log_objects[i]->type, pTemp);
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) 2);
    buf_index = strlen( (const char *)buffer);
    // copy each variable to the buffer.
    for(int j = 0; j < log_objects[i]->num_variables; j++)
    {
      if((buf_index + MAX_LOG_NAME_LENGTH + CTRL_BYTES) > buffer_length)
      {
        return;
      }
      //append value specifier "\v" and update buf_index.
      strncat( (char *) buffer, (const char *) "\\v", (unsigned short) CTRL_BYTES);
      //append group name and update buf_index.
      strncat( (char *) buffer , (const char *) log_objects[i]->var_name[j], (unsigned short) MAX_LOG_NAME_LENGTH);
      buf_index = strlen( (const char *) buffer);
    }
  }
}











