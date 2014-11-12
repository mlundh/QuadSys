///*
// * log.h
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
//#ifndef LOG_H_
//#define LOG_H_
//
//#include "compiler.h"
//
//#define MAX_LOG_VAR (16)
//#define MAX_LOG_NAME_LENGTH (8)
///**
// * Enum describing the available types for log/parameters.
// */
//typedef enum variable_type
//{
//  uint8_variable_type =             1 << 0,
//  uint16_variable_type =            1 << 1,
//  uint32_variable_type =            1 << 2,
//  int8_variable_type =              1 << 3,
//  int16_variable_type =             1 << 4,
//  int32_variable_type =             1 << 5,
//}variable_type_t;
//
//
///**
// * typedef the log_function type.
// */
//typedef uint8_t (*log_function)(log_obj_t log_obj);
//
///**
// * Struct describing a log_obj in the log FW.
// */
//typedef struct log_obj
//{
//  uint8_t num_variables;
//  variable_type_t type;
//  signed char *group_name;
//  log_function function;
//  signed char **var_name;
//  uint8_t *serialized_data[];
//  uint32_t logging_active;
//} log_obj_t;
//
//log_obj_t *create_log_obj(uint8_t num_variables, variable_type_t type, const char *group_name, log_function function);
//void set_log_vaiable_name(log_obj_t *log_obj, const char *name, uint8_t var_index);
//
//void register_log_group(log_obj_t *log_obj);
//
//void serialize_int8(uint8_t **buffer, uint8_t *index);
//void serialize_int16(uint8_t **buffer, uint8_t *index);
//void serialize_int32(uint8_t **buffer, uint8_t *index);
//
//void deserialize_int8(uint8_t **buffer, uint8_t *index);
//void deserialize_int16(uint8_t **buffer, uint8_t *index);
//void deserialize_int32(uint8_t **buffer, uint8_t *index);
//
//#endif /* LOG_H_ */
