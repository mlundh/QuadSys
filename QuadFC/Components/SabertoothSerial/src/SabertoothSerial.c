/*
* SabertoothSerial.c
*
* Copyright (C) 2022  Martin Lundh
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

#include "Components/SabertoothSerial/inc/SabertoothSerial.h"
#include <stddef.h>
#include "HAL/QuadFC/QuadFC_Serial.h"

#define CHECKSUM(buffer) (uint8_t)((buffer[0] + buffer[1] + buffer[2]) & 127)

uint8_t Sabertooth_SendCommand(Sabertooth_t* obj, uint8_t command, uint8_t value)
{
    uint8_t buffer[4];
    buffer[0] = obj->address;
    buffer[1] = command;
    buffer[2] = value;
    buffer[3] = CHECKSUM(buffer);

    QuadFC_Serial_t serial;
    serial.buffer = buffer;
    serial.bufferLength = 4;

    uint8_t returnValue = QuadFC_SerialWrite(&serial,obj->uartNr,50);
    return returnValue;
}


Sabertooth_t* Sabertooth_Create(uint8_t address, int busIndex)
{
    Sabertooth_t* obj = pvPortMalloc(sizeof(Sabertooth_t));
    if(!obj)
    {
        return NULL;
    }
    obj->address = address;
    obj->uartNr = busIndex;
    return obj;
}

uint8_t Sabertooth_DriveMixed(Sabertooth_t* obj, int32_t value)
{
    if(!obj)
    {
        return 0;
    }
    uint8_t command = STC_driveForwardMixed;
    // input in FP16.16 with 1 beeing 100% forward, and -1 100% reverse.
    // value is 16bit, output is 7 bit, shift 9 bits to scale accordingly.
    value = value >> 9; 
    if(value < 0)
    {
        value = -value; // We should reverse, cange sign and change command.
        command = STC_driveBackwardMixed;
    }
    if(value & ~0x7F) // check if we have a value above our max, if so, then clamp to max.
    {
        value = 0x7b;
    }
    
    return Sabertooth_SendCommand(obj, command, value);
}

uint8_t Sabertooth_TurnMixed(Sabertooth_t* obj, int32_t value)
{
    if(!obj)
    {
        return 0;
    }
    uint8_t command = STC_turnRightMixed;
    // input in FP16.16 with 1 beeing 100% right, and -1 100% left.
    // value is 16bit, output is 7 bit, shift 9 bits to scale accordingly.
    value = value >> 9; 
    if(value < 0)
    {
        value = -value; // We should reverse, cange sign and change command.
        command = STC_turnLeftMixed;
    }
    if(value & ~0x7F) // check if we have a value above our max, if so, then clamp to max.
    {
        value = 0x7F;
    }
    return Sabertooth_SendCommand(obj, command, value);

}

uint8_t Sabertooth_DriveM1(Sabertooth_t* obj, int32_t value)
{
    if(!obj)
    {
        return 0;
    }
    uint8_t command = STC_driveForwardM1;
    // input in FP16.16 with 1 beeing 100% forward, and -1 100% reverse.
    // value is 16bit, output is 7 bit, shift 9 bits to scale accordingly.
    value = value >> 9; 
    if(value < 0)
    {
        value = -value; // We should reverse, cange sign and change command.
        command = STC_driveBackwardM1;
    }
    if(value & ~0x7F) // check if we have a value above our max, if so, then clamp to max.
    {
        value = 0x7F;
    }
    return Sabertooth_SendCommand(obj, command, value);
}

uint8_t Sabertooth_DriveM2(Sabertooth_t* obj, int32_t value)
{
    if(!obj)
    {
        return 0;
    }
    uint8_t command = STC_driveForwardM2;
    // input in FP16.16 with 1 beeing 100% forward, and -1 100% reverse.
    // value is 16bit, output is 7 bit, shift 9 bits to scale accordingly.
    value = value >> 9; 
    if(value < 0)
    {
        value = -value; // We should reverse, cange sign and change command.
        command = STC_driveBackwardM2;
    }
    if(value & ~0x7F) // check if we have a value above our max, if so, then clamp to max.
    {
        value = 0x7F;
    }
    return Sabertooth_SendCommand(obj, command, value);
}

uint8_t Sabertooth_SetTimeout(Sabertooth_t* obj, uint8_t timeout)
{
    return Sabertooth_SendCommand(obj, STC_serialTimeout, timeout);
}

uint8_t Sabertooth_SetBaud(Sabertooth_t* obj, SabertoothBaud_t baud)
{
    return Sabertooth_SendCommand(obj, STC_baudRate, baud);

}

uint8_t Sabertooth_SetDeadband(Sabertooth_t* obj, uint8_t deadband)
{
    if(!obj)
    {
        return 0;
    }
    if(deadband > 127) // check if we have a deadband above our max, if so, return an error.
    {
        return 0;
    }
    return Sabertooth_SendCommand(obj, STC_deadband, deadband);
}



