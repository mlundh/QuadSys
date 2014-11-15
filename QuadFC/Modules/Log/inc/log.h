/*
 * log.h
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
#ifndef LOG_H_
#define LOG_H_

#include "stdint.h"
#include "stddef.h"

#define MAX_LOG_GROUPS (32)
#define MAX_LOG_VAR_PER_GROUP (16)
#define MAX_LOG_NAME_LENGTH (8)
#define CTRL_BYTES (2)

/**
 * Enum describing the available types for log/parameters.
 */
typedef enum variable_type
{
  uint8_variable_type =             1 << 0,
  uint16_variable_type =            1 << 1,
  uint32_variable_type =            1 << 2,
  int8_variable_type =              1 << 3,
  int16_variable_type =             1 << 4,
  int32_variable_type =             1 << 5,
}Log_variable_type_t;


/**
 * typedef the log_function type.
 * Example usage:
 *  uint8_t main_log_fcn(const void* log_obj)
 *  {
 *     log_obj_t *log_obj_temp = (log_obj_t*)log_obj;
 *
 *     SerializeInt16(log_obj_temp->serialized_data[0], roll);
 *     SerializeInt16(log_obj_temp->serialized_data[1], pitch);
 *     SerializeInt16(log_obj_temp->serialized_data[2], yaw);
 *     SerializeInt16(log_obj_temp->serialized_data[3], altitude);
 *     return 0;
 *   }
 *
 *
 */
typedef uint8_t (*Log_LogFunction_t)(const void* log_obj);

/**
 * Struct describing a log_obj in the log FW.
 */
typedef struct log_obj
{
  uint8_t num_variables;
  Log_variable_type_t type;
  signed char *group_name;
  Log_LogFunction_t function;
  signed char **var_name;
  uint8_t **serialized_data;
  uint32_t logging_active;
} log_obj_t;


log_obj_t *Log_CreateLogObj(uint8_t num_variables, Log_variable_type_t type, const char *group_name, Log_LogFunction_t function);
void Log_SetLVaiableName(log_obj_t *log_obj, const char *var_name, uint8_t var_index);

void Log_register_log_group(log_obj_t *log_obj);

void Log_serialize_int8(uint8_t **buffer, uint8_t *index);
void Log_serialize_int16(uint8_t **buffer, uint8_t *index);
void Log_serialize_int32(uint8_t **buffer, uint8_t *index);

void Log_deserialize_int8(uint8_t **buffer, uint8_t *index);
void Log_deserialize_int16(uint8_t **buffer, uint8_t *index);
void Log_deserialize_int32(uint8_t **buffer, uint8_t *index);

#endif /* LOG_H_ */
