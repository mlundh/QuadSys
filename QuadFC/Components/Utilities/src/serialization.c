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
#include <string.h>
uint8_t* serialize_uint32_t(uint8_t *buffer, uint32_t* size, uint32_t* value)
{
    if(!buffer || *size < 4)
    {
        return 0;
    }
    buffer[0]  = (uint8_t)((*value >> 24)   & 0xFF);
    buffer[1]  = (uint8_t)((*value >> 16)   & 0xFF);
    buffer[2]  = (uint8_t)((*value >> 8)    & 0xFF);
    buffer[3]  = (uint8_t)((*value)         & 0xFF);
    *size -= 4;
    return buffer + 4;
}

uint8_t* deserialize_uint32_t(uint8_t *buffer, uint32_t* size, uint32_t* value)
{
    if(!buffer || *size < 4)
    {
        return 0;
    }
    *value = (((int32_t)buffer[0]) << 24)+
            (((int32_t)buffer[1]) << 16) +
            (((int32_t)buffer[2]) << 8) +
            ((int32_t)buffer[3]);
    ;
    *size -= 4;
    return buffer + 4;
}

uint8_t* serialize_int32_t(uint8_t *buffer, uint32_t* size, int32_t *value)
{
    return serialize_uint32_t(buffer, size, (uint32_t*)value);
}

uint8_t* deserialize_int32_t(uint8_t *buffer, uint32_t* size, int32_t* value)
{
    return deserialize_uint32_t(buffer, size, (uint32_t*)value);
}

uint8_t* serialize_uint16_t(uint8_t *buffer, uint32_t* size, uint16_t* value)
{
    if(!buffer || *size < 2)
    {
        return 0;
    }
    buffer[0]  = (uint8_t)((*value >> 8)    & 0xFF);
    buffer[1]  = (uint8_t)((*value)         & 0xFF);
    *size -= 2;
    return buffer + 2;
}

uint8_t* deserialize_uint16_t(uint8_t *buffer, uint32_t* size, uint16_t* value)
{
    if(!buffer || *size < 2)
    {
        return 0;
    }
    *value = (((int32_t)buffer[0]) << 8) +
              ((int32_t)buffer[1]);
    ;
    *size -= 2;
    return buffer + 2;
}

uint8_t* serialize_int16_t(uint8_t *buffer, uint32_t* size, int16_t *value)
{
    return serialize_uint16_t(buffer, size, (uint16_t*)value);
}

uint8_t* deserialize_int16_t(uint8_t *buffer, uint32_t* size, int16_t* value)
{
    return deserialize_uint16_t(buffer, size, (uint16_t*)value);
}


uint8_t* serialize_int8_t(uint8_t *buffer, uint32_t* size, int8_t *value)
{
    return serialize_uint8_t(buffer, size, (uint8_t*)value);
}

uint8_t* deserialize_int8_t(uint8_t *buffer, uint32_t* size, int8_t* value)
{
    return deserialize_uint8_t(buffer, size, (uint8_t*)value);
}

uint8_t* serialize_uint8_t(uint8_t *buffer, uint32_t* size, uint8_t *value)
{
    if(!buffer || *size < 4)
    {
        return 0;
    }
    buffer[0]  = (uint8_t)*value;
    *size-=1;
    return buffer + 1;
}

uint8_t* deserialize_uint8_t(uint8_t *buffer, uint32_t* size, uint8_t* value)
{
    if(!buffer || *size < 1)
    {
        return 0;
    }
    *value = ((int8_t)buffer[0]);
    *size -= 1;
    return buffer + 1;
}

uint8_t* serialize_string(uint8_t *buffer, uint32_t* size, uint8_t* string, uint32_t stringLength)
{
    if(!buffer || !string)
    {
        return NULL;
    }
    //ensure that the name will fit in current buffer.
    if((stringLength + 4) > *size)
    {
        return NULL;
    }
    buffer = serialize_uint32_t(buffer, size, &stringLength);
    memcpy(buffer, string, stringLength);
    *size -= stringLength;
    return buffer + stringLength;
}

uint8_t* deserialize_string(uint8_t *buffer, uint32_t* size, uint8_t* string, uint32_t *stringLength, uint32_t stringBufferLength)
{
    if(!buffer || !string)
    {
        return NULL;
    }
    buffer = deserialize_uint32_t(buffer, size, stringLength);

    //ensure that the string will fit in current string buffer.
    if((*stringLength > *size) || (*stringLength > stringBufferLength) || !buffer)
    {
        string = NULL;
        return NULL;
    }
    memcpy(string, buffer, *stringLength);
    *size -= *stringLength;
    return buffer + *stringLength;
}

uint8_t* serialize_control_signal_t(uint8_t *buffer, uint32_t* size, control_signal_t* value)
{
    if(!buffer || *size < (sizeof(int32_t)*4) || !value)
    {
        return NULL;
    }
    buffer = serialize_int32_t(buffer, size, &value->control_signal[0]);
    buffer = serialize_int32_t(buffer, size, &value->control_signal[1]);
    buffer = serialize_int32_t(buffer, size, &value->control_signal[2]);
    buffer = serialize_int32_t(buffer, size, &value->control_signal[3]);

    return buffer;
}
uint8_t* deserialize_control_signal_t(uint8_t *buffer, uint32_t* size, control_signal_t* value)
{
    if(!buffer || *size < (sizeof(int32_t)*4) || !value)
    {
        return NULL;
    }
    buffer = deserialize_int32_t(buffer, size, &value->control_signal[0]);
    buffer = deserialize_int32_t(buffer, size, &value->control_signal[1]);
    buffer = deserialize_int32_t(buffer, size, &value->control_signal[2]);
    buffer = deserialize_int32_t(buffer, size, &value->control_signal[3]);

    return buffer;
}

uint8_t* serialize_genericRC_t(uint8_t *buffer, uint32_t* size, genericRC_t* value)
{
    if(!buffer || *size < (sizeof(int32_t)*4) || !value)
    {
        return NULL;
    }
    buffer = serialize_int32_t(buffer, size, &value->channel[0]);
    buffer = serialize_int32_t(buffer, size, &value->channel[1]);
    buffer = serialize_int32_t(buffer, size, &value->channel[2]);
    buffer = serialize_int32_t(buffer, size, &value->channel[3]);
    buffer = serialize_int32_t(buffer, size, &value->channel[4]);
    buffer = serialize_int32_t(buffer, size, &value->channel[5]);
    buffer = serialize_int32_t(buffer, size, &value->channel[6]);
    buffer = serialize_int32_t(buffer, size, &value->channel[7]);
    buffer = serialize_int32_t(buffer, size, &value->channel[8]);
    buffer = serialize_int32_t(buffer, size, &value->channel[9]);
    buffer = serialize_int32_t(buffer, size, &value->channel[10]);
    buffer = serialize_int32_t(buffer, size, &value->channel[11]);
    

    return buffer;
}
uint8_t* deserialize_genericRC_t(uint8_t *buffer, uint32_t* size, genericRC_t* value)
{
    if(!buffer || *size < (sizeof(int32_t)*4) || !value)
    {
        return NULL;
    }
    buffer = deserialize_int32_t(buffer, size, &value->channel[0]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[1]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[2]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[3]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[4]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[5]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[6]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[7]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[8]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[9]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[10]);
    buffer = deserialize_int32_t(buffer, size, &value->channel[11]);


    return buffer;
}

uint8_t* serialize_state_data_t(uint8_t *buffer, uint32_t* size, state_data_t* value)
{
    if(!buffer || *size < (sizeof(state_data_t)) || !value)
    {
        return NULL;
    }
    for(uint32_t i = 0;i < nr_states_bf; i++)
    {
        buffer = serialize_int32_t(buffer, size, &value->state_bf[i]);
    }
    serialize_uint16_t(buffer, size, &value->state_valid_bf);
    for(uint32_t i = 0;i < nr_states_if; i++)
    {
        buffer = serialize_int32_t(buffer, size, &value->state_bf[i]);
    }
    serialize_uint16_t(buffer, size, &value->state_valid_if);
    return buffer;

}
uint8_t* deserialize_state_data_t(uint8_t *buffer, uint32_t* size, state_data_t* value)
{
    if(!buffer || *size < (sizeof(state_data_t)) || !value)
    {
        return NULL;
    }
    for(uint32_t i = 0;i < nr_states_bf; i++)
    {
        buffer = deserialize_int32_t(buffer, size, &value->state_bf[i]);
    }
    deserialize_uint16_t(buffer, size, &value->state_valid_bf);
    for(uint32_t i = 0;i < nr_states_if; i++)
    {
        buffer = deserialize_int32_t(buffer, size, &value->state_bf[i]);
    }
    deserialize_uint16_t(buffer, size, &value->state_valid_if);
    return buffer;
}

