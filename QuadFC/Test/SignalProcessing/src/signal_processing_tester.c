/*
 * signal_processing_tester.c
 *
 * Copyright (C) 2016 Martin Lundh
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

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <stdio.h>
#include <string.h>
#include "Test/DummyI2C/Dummy_i2c.h"
#include "Test/SignalProcessing/signal_processing_tester.h"
#include "StateEstimator/inc/state_estimator.h"
#include "PortLayer/Communication/inc/satellite_receiver_task.h"
#include "Modules/StateEstimator/inc/signal_processing.h"
#include "FlightController/inc/control_mode_handler.h"

typedef struct SigProcessTester
{
  CtrlModeHandler_t* CtrlModeHandler;
  StateEst_t* stateEst;
} SigProcessTester_t;


uint8_t SigProsses_InitMpu6050(TestFw_t* obj);

void SigProsses_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "ImuToRate", SigProsses_TestImuToRate);
  TestFW_RegisterTest(obj, "Angle", SigProsses_TestAngleAccl);
  TestFW_RegisterTest(obj, "ImuToAngle", SigProsses_TestImuToAngle);
  TestFW_RegisterTest(obj, "ImuToAngle2", SigProsses_TestImuToAngle2);
  TestFW_RegisterTest(obj, "SpectrumToState", SigProcess_TestSpectrumToState);

  // Initialize the handlers and the imu.
  SigProcessTester_t* SigProcessTester = pvPortMalloc(sizeof(SigProcessTester_t));
  SigProcessTester->CtrlModeHandler = Ctrl_CreateModeHandler();
  if(!SigProcessTester->CtrlModeHandler)
  {
    return;
  }
  SigProcessTester->stateEst = StateEst_Create(SigProcessTester->CtrlModeHandler);
  if(!SigProcessTester->stateEst)
  {
    return;
  }
  Ctrl_InitModeHandler(SigProcessTester->CtrlModeHandler);

  TestFW_SetTestSuiteInternal(obj, (void*)SigProcessTester);
  if(!SigProsses_InitMpu6050(obj))
  {
    return;
  }


}

QuadFC_I2C_t* SigProcess_GetI2cPtr(uint8_t internalAddr, uint8_t bufferSize, uint8_t slaveAddr)
{
  QuadFC_I2C_t* i2c_data = malloc(sizeof(QuadFC_I2C_t));
  uint8_t* dataptr = malloc(bufferSize);
  memset(dataptr, 0, bufferSize);
  i2c_data->internalAddr[0] = internalAddr;
  i2c_data->internalAddrLength = 1;
  i2c_data->buffer = dataptr;
  i2c_data->bufferLength = bufferSize;
  i2c_data->slaveAddress = slaveAddr;
  return i2c_data;
}

uint8_t SigProsses_InitMpu6050(TestFw_t* obj)
{
  SigProcessTester_t* SigProcessTester =  (SigProcessTester_t*)TestFW_GetTestSuiteInternal(obj);



  // Add enough data to initialize the imu used (mpu6050 here). Null data
  // is ok for configuration.
  DummyI2C_AddResponse(12, NULL, NULL);

  //Add correct responces for mpu6050_getFullScaleGyroRange.
  QuadFC_I2C_t* i2c_data_FSG = SigProcess_GetI2cPtr(0, 1, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  i2c_data_FSG->buffer[0] = 0x10;
  DummyI2C_AddResponse(1, NULL, i2c_data_FSG);

  // and mpu6050_getFullScaleAccelRange.
  QuadFC_I2C_t* i2c_data_FSA = SigProcess_GetI2cPtr(0, 1, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  i2c_data_FSA->buffer[0] = 0x10;
  DummyI2C_AddResponse(1, NULL, i2c_data_FSA);

  // Dummy reads.
  DummyI2C_AddResponse(100, NULL, NULL);

  // -- SelfTestGyro --
  // get/set FS gyro.
  DummyI2C_AddResponse(3, NULL, NULL);

  // First reading of data. Zero for convenience.
  QuadFC_I2C_t* i2c_data = SigProcess_GetI2cPtr(0x3B /*MPU6050_RA_ACCEL_XOUT_H*/, 14, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(1, NULL, i2c_data);

  // SetSelfTestGyroX,Y,Z on
  DummyI2C_AddResponse(6, NULL, NULL);

  // Second reading of data. Zero for convenience.
  QuadFC_I2C_t* i2c_data2 = SigProcess_GetI2cPtr(0x3B /*MPU6050_RA_ACCEL_XOUT_H*/, 14, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(1, NULL, i2c_data2);


  // SetSelfTestGyroX,Y,Z off
  DummyI2C_AddResponse(6, NULL, NULL);

  // GetGyroX,Y,ZSelfTestFactoryTrim
  QuadFC_I2C_t* i2c_data_FTG = SigProcess_GetI2cPtr(0, 1, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(3, NULL, i2c_data_FTG);

  // reset FS gyro.
  DummyI2C_AddResponse(2, NULL, NULL);

  // -- SelfTestAcc --
  // get/set FS acc.
  DummyI2C_AddResponse(3, NULL, NULL);

  // First reading of data. Zero for convenience.
  QuadFC_I2C_t* i2c_dataAcc = SigProcess_GetI2cPtr(0x3B /*MPU6050_RA_ACCEL_XOUT_H*/, 14, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(1, NULL, i2c_dataAcc);

  // SetSelfTestAccelX,Y,Z on
  DummyI2C_AddResponse(6, NULL, NULL);

  // Second reading of data. Zero for convenience.
  QuadFC_I2C_t* i2c_dataAcc2 = SigProcess_GetI2cPtr(0x3B /*MPU6050_RA_ACCEL_XOUT_H*/, 14, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(1, NULL, i2c_dataAcc2);


  // SetSelfTestAccelX,Y,Z off
  DummyI2C_AddResponse(6, NULL, NULL);

  // GetAccelX,Y,ZSelfTestFactoryTrim
  QuadFC_I2C_t* i2c_data_FTA = SigProcess_GetI2cPtr(0, 1, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(6, NULL, i2c_data_FTA);

  // reset FS Accel.
  DummyI2C_AddResponse(2, NULL, NULL);

  //--------------
  // Add data describing the offset. Offset is zero here.
  QuadFC_I2C_t* i2c_data_offset = SigProcess_GetI2cPtr(0x3B /*MPU6050_RA_ACCEL_XOUT_H*/, 14, 0x69/*MPU6050_ADDRESS_DORTEK*/);
  DummyI2C_AddResponse(1000, NULL, i2c_data_offset);


  //TODO add data for factory trim.
  if(!StateEst_init(SigProcessTester->stateEst))
  {
    return 0;
  }
  return 1;
}

uint8_t SigProsses_TestImuToRate(TestFw_t* obj)
{
  SigProcessTester_t* SigProcessTester =  (SigProcessTester_t*)TestFW_GetTestSuiteInternal(obj);

  StateEst_t* stateEst = SigProcessTester->stateEst;
  if(!stateEst)
  {
    return 0;
  }


  // We have successfully initialized the fake IMU. Now do testing to
  // make sure that the data path from IMU to state vector works
  // correctly for rate mode.

  state_data_t* state = pvPortMalloc( sizeof(state_data_t) );

  QuadFC_I2C_t* i2c_data = SigProcess_GetI2cPtr(0x3B /*MPU6050_RA_ACCEL_XOUT_H*/, 14, 0x69/*MPU6050_ADDRESS_DORTEK*/);

  // Populate the data field. We use 1pi rad per second as the testcase.
  // mpu6050 resolution is: (1000*pi/180/(2^15))rad.
  // 1pi rad in mpu6050 scale is thus: 5898
  uint16_t onePiRad = 5898;
  uint16_t minusOnePiRad = -onePiRad;
  i2c_data->buffer[0] = 0x0;     //  high byte accelerometer x
  i2c_data->buffer[1] = 0x0;     //  low  byte accelerometer x
  i2c_data->buffer[2] = 0x0;     //  high byte accelerometer y
  i2c_data->buffer[3] = 0x0;     //  low  byte accelerometer y
  i2c_data->buffer[4] = 0x0;     //  high byte accelerometer z
  i2c_data->buffer[5] = 0x0;     //  low  byte accelerometer z
  i2c_data->buffer[6] = 0x0;     //  high byte temo
  i2c_data->buffer[7] = 0x0;     //  low  byte temp
  i2c_data->buffer[8] =  (uint8_t)(onePiRad >> 8);      //  high byte gyro x
  i2c_data->buffer[9] =  (uint8_t)onePiRad;             //  low  byte gyro x
  i2c_data->buffer[10] = (uint8_t)(minusOnePiRad >> 8); //  high byte gyro y
  i2c_data->buffer[11] = (uint8_t)minusOnePiRad;        //  low  byte gyro y
  i2c_data->buffer[12] = (uint8_t)(onePiRad >> 8);      //  high byte gyro z
  i2c_data->buffer[13] = (uint8_t)onePiRad;             //  low  byte gyro z

  DummyI2C_AddResponse(1, 0, i2c_data);

  if(!StateEst_getState(stateEst, state))
  {
    return 0;
  }
  //Compare with expected state. Expected state is 1<<16. Add some
  // head room for rounding errors(pi rad is not possible to
  // express exactly in mpu6050 scale).
  if(state->state_bf[roll_rate_bf] >= ((1<<16)-5) &&
      state->state_bf[roll_rate_bf] <= ((1<<16)+5) &&
      state->state_bf[pitch_rate_bf]  >= -((1<<16)+5) &&
      state->state_bf[pitch_rate_bf]  <= -((1<<16)-5) &&
      state->state_bf[yaw_rate_bf]   >= ((1<<16)-5) &&
      state->state_bf[yaw_rate_bf]   <= ((1<<16)+5))
  {
    return 1; // We are within limits.
  }
  return 0;
}

uint8_t SigProsses_TestAngleAccl(TestFw_t* obj)
{

  ImuData_t imuData = {{0}};
  imuData.imu_data[accl_x] = 880;
  imuData.imu_data[accl_y] = 672;
  imuData.imu_data[accl_z] = 64624;

  state_data_t state = {{0}};
  Signal_getEulerAnglesAccel( &state, &imuData);

  //Compare with expected state, 0 rad.
  if(state.state_bf[pitch_bf] >= (0-300) &&
      state.state_bf[pitch_bf] <= (0+300) &&
      state.state_bf[roll_bf]  >= -(0+300) &&
      state.state_bf[roll_bf]  <= -(0+300));
  {
    return 1; // We are within limits.
  }
  return 0;
}

uint8_t SigProsses_TestImuToAngle(TestFw_t* obj)
{
  SigProcessTester_t* SigProcessTester =  (SigProcessTester_t*)TestFW_GetTestSuiteInternal(obj);

  StateEst_t* stateEst = SigProcessTester->stateEst;
  if(!stateEst)
  {
    return 0;
  }
  Ctrl_ChangeMode(SigProcessTester->CtrlModeHandler, Control_mode_attitude);


  // We have successfully initialized the fake IMU. Now do testing to
  // make sure that the data path from IMU to state vector works
  // correctly for attitude mode.

  state_data_t* state = pvPortMalloc( sizeof(state_data_t) );
  // Add data describing the IMU response to a read.
  QuadFC_I2C_t* i2c_data = malloc(sizeof(QuadFC_I2C_t));
  uint8_t* dataptr = malloc(14);
  i2c_data->internalAddr[0] = 0x3B;//MPU6050_RA_ACCEL_XOUT_H;
  i2c_data->internalAddrLength = 1;
  i2c_data->buffer = dataptr;
  i2c_data->bufferLength = 14;
  i2c_data->slaveAddress = 0x69;//MPU6050_ADDRESS_DORTEK

  // Populate the data field.
  // Accelerometer resolution of mpu6050 is
  // (8/(2^(15))) = 0.00024414 g/LSB => 1g = 4096
  //
  // Given test point where the copter is
  // level ( data taken from real copter):
  int32_t testPointAngleX = 55; //297 45 deg
  int32_t testPointAngleY = 42; //2841
  int16_t testPointAngleZ = -4039; //-2993

  dataptr[0] = (uint8_t)(testPointAngleX >> 8);     //  high byte accelerometer x
  dataptr[1] = (uint8_t)testPointAngleX;            //  low  byte accelerometer x
  dataptr[2] = (uint8_t)(testPointAngleY >> 8);     //  high byte accelerometer y
  dataptr[3] = (uint8_t)testPointAngleY;            //  low  byte accelerometer y
  dataptr[4] = (uint8_t)(testPointAngleZ >> 8);     //  high byte accelerometer z
  dataptr[5] = (uint8_t)testPointAngleZ;            //  low  byte accelerometer z
  dataptr[6] = 0x0;                                 //  high byte temp
  dataptr[7] = 0x0;                                 //  low  byte temp
  dataptr[8] =  0x0;                 //  high byte gyro x
  dataptr[9] =  0x0;                 //  low  byte gyro x
  dataptr[10] = 0x0;                 //  high byte gyro y
  dataptr[11] = 0x0;                 //  low  byte gyro y
  dataptr[12] = 0x0;                 //  high byte gyro z
  dataptr[13] = 0x0;                 //  low  byte gyro z

  DummyI2C_AddResponse(1, i2c_data, i2c_data);

  if(!StateEst_getState(stateEst, state))
  {
    return 0;
  }
  // Compare with expected state, 0 rad. Add headroom for rounding
  // errors and since real values was used. 300 in state scale is
  // approximately 0.014 rad ~ 0.8 deg

  if(state->state_bf[pitch_bf] >= (0-300) &&
      state->state_bf[pitch_bf] <= (0+300) &&
      state->state_bf[roll_bf]  >= (0-300) &&
      state->state_bf[roll_bf]  <= (0+300));
  {
    return 1; // We are within limits.
  }
  return 0;
}

uint8_t SigProsses_TestImuToAngle2(TestFw_t* obj)
{
  SigProcessTester_t* SigProcessTester =  (SigProcessTester_t*)TestFW_GetTestSuiteInternal(obj);

  StateEst_t* stateEst = SigProcessTester->stateEst;
  if(!stateEst)
  {
    return 0;
  }
  Ctrl_ChangeMode(SigProcessTester->CtrlModeHandler, Control_mode_attitude);


  // We have successfully initialized the fake IMU. Now do testing to
  // make sure that the data path from IMU to state vector works
  // correctly for attitude mode.

  state_data_t* state = pvPortMalloc( sizeof(state_data_t) );
  // Add data describing the IMU response to a read.
  QuadFC_I2C_t* i2c_data = malloc(sizeof(QuadFC_I2C_t));
  uint8_t* dataptr = malloc(14);
  i2c_data->internalAddr[0] = 0x3B;//MPU6050_RA_ACCEL_XOUT_H;
  i2c_data->internalAddrLength = 1;
  i2c_data->buffer = dataptr;
  i2c_data->bufferLength = 14;
  i2c_data->slaveAddress = 0x69;//MPU6050_ADDRESS_DORTEK

  // Populate the data field.
  // Accelerometer resolution of mpu6050 is
  // (8/(2^(15))) = 0.00024414 g/LSB => 1g = 4096
  //
  // Given test point where the copter is
  // level ( data taken from real copter):
  int32_t testPointAngleX = 297;
  int32_t testPointAngleY = 2841;// 45 deg
  int16_t testPointAngleZ = 2993;

  dataptr[0] = (uint8_t)(testPointAngleX >> 8);     //  high byte accelerometer x
  dataptr[1] = (uint8_t)testPointAngleX;            //  low  byte accelerometer x
  dataptr[2] = (uint8_t)(testPointAngleY >> 8);     //  high byte accelerometer y
  dataptr[3] = (uint8_t)testPointAngleY;            //  low  byte accelerometer y
  dataptr[4] = (uint8_t)(testPointAngleZ >> 8);     //  high byte accelerometer z
  dataptr[5] = (uint8_t)testPointAngleZ;            //  low  byte accelerometer z
  dataptr[6] = 0x0;                                 //  high byte temp
  dataptr[7] = 0x0;                                 //  low  byte temp
  dataptr[8] =  0x0;                 //  high byte gyro x
  dataptr[9] =  0x0;                 //  low  byte gyro x
  dataptr[10] = 0x0;                 //  high byte gyro y
  dataptr[11] = 0x0;                 //  low  byte gyro y
  dataptr[12] = 0x0;                 //  high byte gyro z
  dataptr[13] = 0x0;                 //  low  byte gyro z

  DummyI2C_AddResponse(1, i2c_data, i2c_data);

  if(!StateEst_getState(stateEst, state))
  {
    return 0;
  }
  // Compare with expected state, roll 0 rad, pitch pi rad. Add headroom for rounding
  // errors and since real values was used. 300 in state scale is
  // approximately 0.014 rad ~ 0.8 deg

  if(state->state_bf[pitch_bf] >= ((1<<16)/4-300) && // pi rad
      state->state_bf[pitch_bf] <= ((1<<16)/4-300) &&
      state->state_bf[roll_bf]  >= (0-300) && // 0 rad
      state->state_bf[roll_bf]  <= (0+300));
  {
    return 1; // We are within limits.
  }
  return 0;
}


uint8_t SigProcess_TestSpectrumToState(TestFw_t* obj)
{
  spektrum_data_t receiver_data = {{{0}}};
  state_data_t state_data = {{0}};

  // Warning! Do not do this in real code. Use the Satellite_init function.
  // Here we only use the divisor and multiplier fields, so we populate them
  // Separately.
  Satellite_t satelite_obj = {0};
  satelite_obj.divisor = 1;
  satelite_obj.multiplier = 1;
  satelite_obj.throMult = 1;
  satelite_obj.xMutexParam = xSemaphoreCreateMutex();
  //Populate the receiver_data with known data.
  // 1 pi rad/s in receiver resolution (11 bit) = 512
  // but receiver has center at 1022, so add this.
  receiver_data.ch[0].value = (uint16_t)2047; // THRO is treated differently, 100% = 2048
  receiver_data.ch[1].value = (uint16_t)(512 + SATELLITE_CH_CENTER); // ROLL
  receiver_data.ch[2].value = (uint16_t)(512 + SATELLITE_CH_CENTER); // PITCH
  receiver_data.ch[3].value = (uint16_t)(-512 + SATELLITE_CH_CENTER); // YAW tests a negative value.


  Satellite_MapToSetpoint(&satelite_obj, &receiver_data, &state_data);

  //Compare with expected state. Expected state is 1<<16. Add some
  // head room for rounding errors.
  if(state_data.state_bf[pitch_rate_bf] >=  ((1<<16)-1) &&
      state_data.state_bf[pitch_rate_bf] <=  ((1<<16)+1) &&
      state_data.state_bf[roll_rate_bf]  >=  ((1<<16)-1) &&
      state_data.state_bf[roll_rate_bf]  <=  ((1<<16)+1) &&
      state_data.state_bf[yaw_rate_bf]   >= -((1<<16)+1) &&
      state_data.state_bf[yaw_rate_bf]   <= -((1<<16)-1) &&
      state_data.state_bf[thrust_sp]     >= -((1<<30)-200) &&
      state_data.state_bf[thrust_sp]     <=  ((1<<30)+200))
  {
    return 1; // We are within limits.
  }
  return 0;
}

