/*
 * serialization.h
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
#ifndef MODULES_UTILITIES_INC_SERIALIZATION_H_
#define MODULES_UTILITIES_INC_SERIALIZATION_H_

#include <stdint.h>
#include <stddef.h>
#include "Messages/inc/common_types.h"
/**
 * Serialize the value pointed to by value.
 * @param buffer    Pointer to the buffer where the serialized value will be stored.
 * @param size      Pointer to the size of the buffer. This will be updated to reflect
 *                  the remaining size after writing the data.
 * @param value     Pointer to the value to serialize. Used a pointer so that the interface
 *                  is similar for simple and complex types.
 * @return          Pointer to the end of the written data. Use this together with the size
 *                  parameter to continue the serialization.
 */
uint8_t* serialize_uint32_t(uint8_t *buffer, uint32_t* size, uint32_t *value);
uint8_t* deserialize_uint32_t(uint8_t *buffer, uint32_t* size, uint32_t* value);

uint8_t* serialize_int32_t(uint8_t *buffer, uint32_t* size, int32_t *value);
uint8_t* deserialize_int32_t(uint8_t *buffer, uint32_t* size, int32_t* value);

uint8_t* serialize_uint16_t(uint8_t *buffer, uint32_t* size, uint16_t* value);
uint8_t* deserialize_uint16_t(uint8_t *buffer, uint32_t* size, uint16_t* value);

uint8_t* serialize_int16_t(uint8_t *buffer, uint32_t* size, int16_t *value);
uint8_t* deserialize_int16_t(uint8_t *buffer, uint32_t* size, int16_t* value);

uint8_t* serialize_uint8_t(uint8_t *buffer, uint32_t* size, uint8_t *value);
uint8_t* deserialize_uint8_t(uint8_t *buffer, uint32_t* size, uint8_t* value);

uint8_t* serialize_int8_t(uint8_t *buffer, uint32_t* size, int8_t *value);
uint8_t* deserialize_int8_t(uint8_t *buffer, uint32_t* size, int8_t* value);

uint8_t* serialize_string(uint8_t *buffer, uint32_t* size, uint8_t* string, uint32_t stringLength);
uint8_t* deserialize_string(uint8_t *buffer, uint32_t* size, uint8_t* string, uint32_t *stringLength, uint32_t stringBufferLength);

uint8_t* serialize_control_signal_t(uint8_t *buffer, uint32_t* size, control_signal_t* value);
uint8_t* deserialize_control_signal_t(uint8_t *buffer, uint32_t* size, control_signal_t* value);

uint8_t* serialize_genericRC_t(uint8_t *buffer, uint32_t* size, genericRC_t* value);
uint8_t* deserialize_genericRC_t(uint8_t *buffer, uint32_t* size, genericRC_t* value);


uint8_t* serialize_state_data_t(uint8_t *buffer, uint32_t* size, state_data_t* value);
uint8_t* deserialize_state_data_t(uint8_t *buffer, uint32_t* size, state_data_t* value);

#endif /* MODULES_UTILITIES_INC_SERIALIZATION_H_ */
