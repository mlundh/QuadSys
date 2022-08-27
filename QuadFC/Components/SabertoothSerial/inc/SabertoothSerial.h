/*
* SabertoothSerial.h
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
#ifndef __SABERTOOTHSERIAL_H__
#define __SABERTOOTHSERIAL_H__
#include "stdint.h"

typedef struct Sabertooth
{
    uint8_t address;
    uint8_t uartNr;
} Sabertooth_t;

typedef struct SabertoothPacket
{
    uint8_t address;
    uint8_t command;
    uint8_t data;
    uint8_t checksum; // (address + command + data) & 127
} SabertoothPacket_t;

typedef enum SabertoothBaud
{
    STB_2400 = 1,
    STB_9600 = 2,
    STB_19200 = 3,
    STB_38400 = 4,
} SabertoothBaud_t;

typedef enum SabertoothCommand
{  
    STC_driveForwardM1 = 0,
    STC_driveBackwardM1 = 1,
    STC_MinVolage = 2,
    STC_MaxVoltage = 3,
    STC_driveForwardM2 = 4,
    STC_driveBackwardM2 = 5,
    STC_drive7BitM1 = 6,
    STC_drive7BitM2 = 7,

    STC_driveForwardMixed = 8,
    STC_driveBackwardMixed = 9,
    STC_turnRightMixed = 10,
    STC_turnLeftMixed = 11,
    STC_drive7BitMixed = 12,
    STC_turn7BitMixed = 13,

    STC_serialTimeout = 14,
    STC_baudRate = 15,
    STC_ramping = 16,
    STC_deadband = 17,
}SabertoothCommand_t;

/**
 * @brief Create the sabertooth instance. 
 * 
 * @param address   Address of the controller, [128,135]
 * @param busIndex  Bus index as defined by hw layout.
 * @return Sabertooth_t* 
 */
Sabertooth_t* Sabertooth_Create(uint8_t address, int busIndex);

/**
 * @brief Drive in mixed(tank) mode. 
 * 
 * FP16.16 
 * 1=full speed forward, 
 * -1=full reverse, 
 * 0=stop.
 * 
 * @param obj           Sabertooth object
 * @param value         Value as defined above.
 * @return uint8_t      0 if fail, 1 otherwise
 */
uint8_t Sabertooth_DriveMixed(Sabertooth_t* obj, int32_t value);

/**
 * @brief Turn in mixed(tank) mode. 
 * 
 * FP16.16 
 * 1=full right turn, 
 * -1=full left turn, 
 * 0 = no turn.
 * 
 * @param obj           Sabertooth object
 * @param value         Value as defined above.
 * @return uint8_t      0 if fail, 1 otherwise
 */
uint8_t Sabertooth_TurnMixed(Sabertooth_t* obj, int32_t value);


/**
 * @brief Drive motor 1. 
 * 
 * FP16.16 
 * 1=full forward, 
 * -1=full reverse. 
 * 0 = stop.
 * 
 * @param obj           Sabertooth object
 * @param value         Value as defined above.
 * @return uint8_t      0 if fail, 1 otherwise
 */
uint8_t Sabertooth_DriveM1(Sabertooth_t* obj, int32_t value);

/**
 * @brief Drive motor 2. 
 * 
 * FP16.16 
 * 1=full forward, 
 * -1=full reverse. 
 * 0 = stop.
 * 
 * @param obj           Sabertooth object
 * @param value         Value as defined above.
 * @return uint8_t      0 if fail, 1 otherwise
 */
uint8_t Sabertooth_DriveM2(Sabertooth_t* obj, int32_t value);

/**
 * @brief Set setpoint timeout
 * 
 * The timeout scales 1 unit per 100ms of timeout. 
 * 
 * A command of 0 will disable the timeout if
 * you had previously enabled it.
 * 
 * @param obj       Sabertooth object
 * @param timeout   Timeout value, 0 = no timeout.
 * @return uint8_t  0 if fail, 1 otherwise.
 */
uint8_t Sabertooth_SetTimeout(Sabertooth_t* obj, uint8_t timeout);

/**
 * @brief Set a new baudrate to the controller.
 * 
 * @param obj       Sabertooth object
 * @param baud      New baud according to the baud enum.
 * @return uint8_t  0 if fail, 1 otherwise.
 */
uint8_t Sabertooth_SetBaud(Sabertooth_t* obj, SabertoothBaud_t baud);

/**
 * @brief Set deadband for the controller. 
 * 
 * Value range [0,127]
 * 
 * 127-command < motors off < 128+command
 * 
 * A command of 3 would shut the motors off with speed commands between 124 and 131
 * 
 * @param obj 
 * @param deadband 
 * @return uint8_t 
 */
uint8_t Sabertooth_SetDeadband(Sabertooth_t* obj, uint8_t deadband);




#endif // __SABERTOOTHSERIAL_H__
