///*
// * log.c
// *
// * Copyright (C) 2014 martin
// *
// * Permission is hereby granted, free of charge, to any person obtaining a copy
// * of this software and associated documentation files (the "Software"), to deal
// * in the Software without restriction, including without limitation the rights
// * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// * copies of the Software, and to permit persons to whom the Software is
// * furnished to do so, subject to the following conditions:
// *
// * The above copyright notice and this permission notice shall be included in
// * all copies or substantial portions of the Software.
// *
// * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// * THE SOFTWARE.
// */
//
//#include "log.h"
//#include "string.h"
///* Kernel includes. */
//#include "FreeRTOS.h"
//
//
//log_obj_t *create_log_obj(uint8_t num_variables, variable_type_t type, const char *group_name, log_function function)
//{
//  // Do some input checks.
//  if(num_variables > MAX_LOG_VAR || function == NULL)
//  {
//    return NULL;
//  }
//  // Some more checking and type identification.
//  int size = 0;
//  switch ( type )
//  {
//  case uint8_variable_type:
//  case int8_variable_type:
//    size = (sizeof( uint8_t ) * num_variables);
//    break;
//  case uint16_variable_type:
//  case int16_variable_type:
//    size = (sizeof( uint16_t ) * num_variables);
//    break;
//  case uint32_variable_type:
//  case int32_variable_type:
//    size = (sizeof( uint32_t ) * num_variables);
//    break;
//  default:
//    return NULL;
//  }
//
//  log_obj_t *log_obj = pvPortMalloc( sizeof(log_obj_t) );
//
//  log_obj->num_variables = num_variables;
//  log_obj->type = type;
//  log_obj->group_name = pvPortMalloc( sizeof( unsigned char ) * MAX_LOG_NAME_LENGTH );
//  log_obj->function = function;
//  log_obj->logging_active = 0;
//  for(int i = 0; i < num_variables; i++)
//  {
//    //alloc and set mem to 0;
//    log_obj->var_name[i] = pvPortMalloc( (sizeof( unsigned char )) * MAX_LOG_NAME_LENGTH );
//    memset(( char * ) log_obj->var_name[i], 0, ( unsigned short ) MAX_LOG_NAME_LENGTH);
//  }
//  strncpy( ( char * ) log_obj->group_name, ( const char * ) group_name, ( unsigned short ) MAX_LOG_NAME_LENGTH );
//
//
//  for(int i = 0; i < num_variables; i++)
//  {
//    //alloc and set mem to 0;
//    log_obj->serialized_data[i] = pvPortMalloc( size );
//    memset(( char * ) log_obj->serialized_data[i], 0, size);
//  }
//  return log_obj;
//}
//
//
//void set_log_vaiable_name(log_obj_t *log_obj, const char *var_name, uint8_t var_index)
//{
//  if(var_index >= log_obj->num_variables)
//  {
//    return;
//  }
//  strncpy( ( char * ) log_obj->var_name[var_index], ( const char * ) var_name, ( unsigned short ) MAX_LOG_NAME_LENGTH );
//}
