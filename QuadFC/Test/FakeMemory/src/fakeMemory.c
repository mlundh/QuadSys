/*
 * fakeMemory.c
 *
 * Copyright (C) 2019 Martin Lundh
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




#include "HAL/QuadFC/QuadFC_Memory.h"
#include <string.h>

#define FAKE_MEM_SIZE (5000)

uint8_t memory[FAKE_MEM_SIZE];

uint8_t Mem_Init()
{
    return 1;
}

uint8_t Mem_Read(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
    if(addr + size < FAKE_MEM_SIZE)
    {
        if(size < buffer_size)
        {
            memcpy(buffer, memory + addr, size);
            return 1;
        }
    }
    return 0;
}

uint8_t Mem_Write(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
    if(addr + size < FAKE_MEM_SIZE)
    {
        if(size < buffer_size)
        {
            memcpy(memory + addr, buffer, size);
            return 1;
        }
    }
    return 0;
}
