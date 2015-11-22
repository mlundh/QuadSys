/*
 * mpu6050.c
 *
 * Created: 2013-04-28 13:46:29
 *  Author: Martin Lundh
 * Code based on Jeff Rowbergs i2cdevlib for the MPU6050, see copyright notice below for details.
 */

// I2Cdev library collection - MPU6050 I2C device class
// Based on InvenSense MPU-6050 register map document rev. 2.0, 5/19/2011 (RM-MPU-6000A-00)
// 8/24/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//     ... - ongoing debug release
// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.
/* ============================================
 I2Cdev device library code is placed under the MIT license
 Copyright (c) 2012 Jeff Rowberg

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ===============================================
 */
#include "string.h"
#include "mpu6050.h"
#include "QuadFC_Peripherals.h"
#include "globals.h"
#include "parameters.h"

#define MPU6050_BUSS                  (0x0)
#define MPU6050_BLOCK_TIME            (50UL / portTICK_PERIOD_MS)

typedef struct ImuInternals
{
  uint8_t i2cBus;
  uint8_t slaveAddress;
  uint8_t rawData[14];
  SemaphoreHandle_t xMutexParam;

}ImuInternals_t;

ImuInternals_t *Imu_getInternals(Imu_t *obj);

ImuInternals_t *Imu_getInternals(Imu_t *obj)
 {
   return (ImuInternals_t *)obj->internals;
 }

Imu_t * Imu_Create()
{
  Imu_t *ImuObj = pvPortMalloc(sizeof(Imu_t));

  ImuObj->internals = pvPortMalloc( sizeof(ImuInternals_t) );
  ImuInternals_t *internals = Imu_getInternals(ImuObj);

  internals->xMutexParam = xSemaphoreCreateMutex();

  param_obj_t * ImuRoot = Param_CreateObj(6, NoType, readOnly, NULL, "IMU_Off", Param_GetRoot(), NULL);

  Param_CreateObj(0, int16_variable_type, readOnly,
      &ImuObj->ImuOffset.imu_data[0], "accl_x", ImuRoot, internals->xMutexParam);
  Param_CreateObj(0, int16_variable_type, readOnly,
      &ImuObj->ImuOffset.imu_data[1], "accl_y", ImuRoot, internals->xMutexParam);
  Param_CreateObj(0, int16_variable_type, readOnly,
      &ImuObj->ImuOffset.imu_data[2], "accl_z", ImuRoot, internals->xMutexParam);
  Param_CreateObj(0, int16_variable_type, readOnly,
      &ImuObj->ImuOffset.imu_data[3], "gyro_x", ImuRoot, internals->xMutexParam);
  Param_CreateObj(0, int16_variable_type, readOnly,
      &ImuObj->ImuOffset.imu_data[4], "gyro_y", ImuRoot, internals->xMutexParam);
  Param_CreateObj(0, int16_variable_type, readOnly,
      &ImuObj->ImuOffset.imu_data[5], "gyro_z", ImuRoot, internals->xMutexParam);

  return ImuObj;
}


uint8_t Imu_Init(Imu_t *obj)
{
  static TickType_t delay_1000_ms = (1000UL / portTICK_PERIOD_MS); /*delay of 1000ms*/

  ImuInternals_t *internals = Imu_getInternals(obj);

  internals->i2cBus = MPU6050_BUSS;
  internals->slaveAddress = MPU6050_ADDRESS_DORTEK;
  ImuData_t  tmp =  {{0}};

  memcpy(&obj->ImuOffset, &tmp, sizeof(tmp));
  memcpy(&obj->ImuData, &tmp, sizeof(tmp));

  mpu6050_reset(obj);
  vTaskDelay( delay_1000_ms );

  mpu6050_setClockSource(obj, MPU6050_CLOCK_PLL_XGYRO);
  mpu6050_setFullScaleGyroRange(obj, MPU6050_GYRO_FS_500);
  mpu6050_setFullScaleAccelRange(obj, MPU6050_ACCEL_FS_16);
  mpu6050_setSleepEnabled(obj, 0);
  mpu6050_setDLPFMode(obj, MPU6050_DLPF_BW_42);

  vTaskDelay( delay_1000_ms );


  /*Calculate offsets for IMU.*/
  uint8_t result = 0;
  result = mpu6050_calc_offset(obj);

  return result;
}

uint8_t Imu_GetData(Imu_t *obj)
{
  ImuInternals_t *internals = Imu_getInternals(obj);

  QuadFC_I2C i2c_data;
  i2c_data.internalAddr[0] = MPU6050_RA_ACCEL_XOUT_H;
  i2c_data.internalAddrLength = 1;
  i2c_data.buffer = internals->rawData;
  i2c_data.bufferLength = 14;
  i2c_data.slaveAddress = internals->slaveAddress;

  if(!QuadFC_i2cRead(&i2c_data, internals->i2cBus, MPU6050_BLOCK_TIME))
  {
    return 0;
  }
  obj->ImuData.imu_data[accl_x]      = (int16_t) ((i2c_data.buffer[0]     << 8) | i2c_data.buffer[1]);
  obj->ImuData.imu_data[accl_y]      = (int16_t) ((i2c_data.buffer[2]     << 8) | i2c_data.buffer[3]);
  obj->ImuData.imu_data[accl_z]      = (int16_t) ((i2c_data.buffer[4]     << 8) | i2c_data.buffer[5]);
  obj->ImuData.imu_data[temp_imu]    = (int16_t) ((i2c_data.buffer[6]     << 8) | i2c_data.buffer[7]);
  obj->ImuData.imu_data[gyro_x]      = -((int16_t) ((i2c_data.buffer[8]   << 8) | i2c_data.buffer[9]))   - obj->ImuOffset.imu_data[gyro_x];
  obj->ImuData.imu_data[gyro_y]      =   (int16_t) ((i2c_data.buffer[10]  << 8) | i2c_data.buffer[11])  - obj->ImuOffset.imu_data[gyro_y];
  obj->ImuData.imu_data[gyro_z]      = -((int16_t) ((i2c_data.buffer[12]  << 8) | i2c_data.buffer[13]))  - obj->ImuOffset.imu_data[gyro_z];
  return 1;
}


/**
 *
 * @param reg_addr  Register address
 * @param bit_nr    bitnumber to change
 * @param nr_bits
 * @param data
 * @param nr_bytes
 * @return
 */
uint8_t mpu6050_write_settings(Imu_t *obj, uint8_t reg_addr, uint8_t bit_nr, uint8_t nr_bits, uint8_t data, uint8_t nr_bytes )
{

  ImuInternals_t *internals = Imu_getInternals(obj);

  uint8_t current_reg_value = 0;

  mpu6050_read_byte(obj, reg_addr, &current_reg_value );

  uint8_t mask = ((1 << nr_bits) - 1) << (bit_nr - nr_bits + 1); // create mask.

  data <<= (bit_nr - nr_bits + 1);  // shift data into correct position
  data &= mask;                     // zero all non-important bits in data
  current_reg_value &= ~(mask);     // zero all important bits in existing word
  current_reg_value |= data;        // combine data with existing word

  data = (current_reg_value & 0xFF);

  QuadFC_I2C i2c_data;
  i2c_data.internalAddr[0] = reg_addr;
  i2c_data.internalAddrLength = 1;
  i2c_data.buffer = &data;
  i2c_data.bufferLength = 1;
  i2c_data.slaveAddress = internals->slaveAddress;

  if(!QuadFC_i2cWrite(&i2c_data, internals->i2cBus, MPU6050_BLOCK_TIME))
  {
    return 0;
  }
  return 1;
}

uint8_t mpu6050_read_settings(Imu_t *obj, uint8_t reg_addr, uint8_t bit_nr, uint8_t nr_bits, uint8_t *data, uint8_t nr_bytes )
{
  ImuInternals_t *internals = Imu_getInternals(obj);

  uint8_t readData[nr_bytes];
  QuadFC_I2C i2c_data;
  i2c_data.internalAddr[0] = reg_addr;
  i2c_data.internalAddrLength = 1;
  i2c_data.buffer = readData;
  i2c_data.bufferLength = nr_bytes;
  i2c_data.slaveAddress = internals->slaveAddress;

  if(!QuadFC_i2cRead(&i2c_data, internals->i2cBus, MPU6050_BLOCK_TIME))
  {
    return 0;
  }

  uint8_t w = *i2c_data.buffer;
  uint8_t mask = ((1 << nr_bits) - 1) << (bit_nr - nr_bits + 1);
  w &= mask;
  w >>= (bit_nr - nr_bits + 1);
  *data = w;

  return 1;
}


uint8_t mpu6050_calc_offset(Imu_t * obj)
{
  const TickType_t xPeriod = (2UL / portTICK_PERIOD_MS);

  int i = 0;
  /*The first readings might be nonsense*/
  for (i = 0; i < 100; i++)
  {
    if(!Imu_GetData(obj))
    {
      return 0;
    }
    vTaskDelay(xPeriod);
  }

  int64_t temp_accl_x = 0;
  int64_t temp_accl_y = 0;
  int64_t temp_accl_z = 0;
  int64_t temp_gyro_x = 0;
  int64_t temp_gyro_y = 0;
  int64_t temp_gyro_z = 0;

  for (i = 0; i < 1000; i++)
  {
    if(!Imu_GetData(obj))
    {
      return 0;
    }
    temp_accl_x += (int64_t) obj->ImuData.imu_data[accl_x];
    temp_accl_y += (int64_t) obj->ImuData.imu_data[accl_y];
    temp_accl_z += (int64_t) obj->ImuData.imu_data[accl_z];
    temp_gyro_x += (int64_t) obj->ImuData.imu_data[gyro_x];
    temp_gyro_y += (int64_t) obj->ImuData.imu_data[gyro_y];
    temp_gyro_z += (int64_t) obj->ImuData.imu_data[gyro_z];
    // TODO check validity of measurments!!
    vTaskDelay( xPeriod );

  }

  obj->ImuOffset.imu_data[accl_x] = (int16_t) (temp_accl_x / 1000);
  obj->ImuOffset.imu_data[accl_y] = (int16_t) (temp_accl_y / 1000);
  obj->ImuOffset.imu_data[accl_z] = (int16_t) (temp_accl_z / 1000);
  obj->ImuOffset.imu_data[gyro_x] = (int16_t) (temp_gyro_x / 1000);
  obj->ImuOffset.imu_data[gyro_y] = (int16_t) (temp_gyro_y / 1000);
  obj->ImuOffset.imu_data[gyro_z] = (int16_t) (temp_gyro_z / 1000);

  return 1;
}



/** Set clock source setting.
 * An internal 8MHz oscillator, gyroscope based clock, or external sources can
 * be selected as the MPU-60X0 clock source. When the internal 8 MHz oscillator
 * or an external source is chosen as the clock source, the MPU-60X0 can operate
 * in low power modes with the gyroscopes disabled.
 *
 * Upon power up, the MPU-60X0 clock source defaults to the internal oscillator.
 * However, it is highly recommended that the device be configured to use one of
 * the gyroscopes (or an external clock source) as the clock reference for
 * improved stability. The clock source can be selected according to the following table:
 *
 * <pre>
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillator
 * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
 * </pre>
 *
 * @param source New clock source setting
 * @see getClockSource()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_CLKSEL_BIT
 * @see MPU6050_PWR1_CLKSEL_LENGTH
 */
void mpu6050_setClockSource(Imu_t *obj, uint8_t source )
{
  mpu6050_write_settings(obj, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source, 1 );
}

/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
void mpu6050_setFullScaleGyroRange(Imu_t *obj, uint8_t range )
{
  mpu6050_write_settings(obj, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range, 1 );
}

/** Get full-scale accelerometer range.
 * The FS_SEL parameter allows setting the full-scale range of the accelerometer
 * sensors, as described in the table below.
 *
 * <pre>
 * 0 = +/- 2g
 * 1 = +/- 4g
 * 2 = +/- 8g
 * 3 = +/- 16g
 * </pre>
 *
 * @return Current full-scale accelerometer range setting
 * @see MPU6050_ACCEL_FS_2
 * @see MPU6050_RA_ACCEL_CONFIG
 * @see MPU6050_ACONFIG_AFS_SEL_BIT
 * @see MPU6050_ACONFIG_AFS_SEL_LENGTH
 */
void mpu6050_getFullScaleAccelRange(Imu_t *obj, uint8_t *buffer )
{
  mpu6050_read_settings(obj, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, buffer, 1 );

}

/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 */
void mpu6050_setFullScaleAccelRange(Imu_t *obj, uint8_t range )
{
  mpu6050_write_settings(obj, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range, 1 );
}

/** Get sleep mode status.
 * Setting the SLEEP bit in the register puts the device into very low power
 * sleep mode. In this mode, only the serial interface and internal registers
 * remain active, allowing for a very low standby current. Clearing this bit
 * puts the device back into normal mode. To save power, the individual standby
 * selections for each of the gyros should be used if any gyro axis is not used
 * by the application.
 * @return Current sleep mode enabled status
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
void mpu6050_getSleepEnabled(Imu_t *obj, uint8_t *buffer )
{
  mpu6050_read_bit(obj, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, buffer );
}
/** Set sleep mode status.
 * @param enabled New sleep mode enabled status
 * @see getSleepEnabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
void mpu6050_setSleepEnabled(Imu_t *obj, uint8_t enabled )
{
  mpu6050_write_bit(obj, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled );
}

/** Get digital low-pass filter configuration.
 * The DLPF_CFG parameter sets the digital low pass filter configuration. It
 * also determines the internal sampling rate used by the device as shown in
 * the table below.
 *
 * Note: The accelerometer output rate is 1kHz. This means that for a Sample
 * Rate greater than 1kHz, the same accelerometer sample may be output to the
 * FIFO, DMP, and sensor registers more than once.
 *
 * <pre>
 *          |   ACCELEROMETER    |           GYROSCOPE
 * DLPF_CFG | Bandwidth | Delay  | Bandwidth | Delay  | Sample Rate
 * ---------+-----------+--------+-----------+--------+-------------
 * 0        | 260Hz     | 0ms    | 256Hz     | 0.98ms | 8kHz
 * 1        | 184Hz     | 2.0ms  | 188Hz     | 1.9ms  | 1kHz
 * 2        | 94Hz      | 3.0ms  | 98Hz      | 2.8ms  | 1kHz
 * 3        | 44Hz      | 4.9ms  | 42Hz      | 4.8ms  | 1kHz
 * 4        | 21Hz      | 8.5ms  | 20Hz      | 8.3ms  | 1kHz
 * 5        | 10Hz      | 13.8ms | 10Hz      | 13.4ms | 1kHz
 * 6        | 5Hz       | 19.0ms | 5Hz       | 18.6ms | 1kHz
 * 7        |   -- Reserved --   |   -- Reserved --   | Reserved
 * </pre>
 *
 * @return DLFP configuration
 * @see MPU6050_RA_CONFIG
 * @see MPU6050_CFG_DLPF_CFG_BIT
 * @see MPU6050_CFG_DLPF_CFG_LENGTH
 */
uint8_t mpu6050_getDLPFMode(Imu_t *obj, uint8_t *buffer )
{
  mpu6050_read_settings(obj, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, buffer, 1 );
  return buffer[0];
}
/** Set digital low-pass filter configuration.
 * @param mode New DLFP configuration setting
 * @see getDLPFBandwidth()
 * @see MPU6050_DLPF_BW_256
 * @see MPU6050_RA_CONFIG
 * @see MPU6050_CFG_DLPF_CFG_BIT
 * @see MPU6050_CFG_DLPF_CFG_LENGTH
 */
void mpu6050_setDLPFMode(Imu_t *obj, uint8_t mode )
{
  mpu6050_write_settings(obj, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode, 1 );
}

// PWR_MGMT_1 register

/** Trigger a full device reset.
 * A small delay of ~50ms may be desirable after triggering a reset.
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_DEVICE_RESET_BIT
 */
void mpu6050_reset(Imu_t *obj)
{
  mpu6050_write_bit(obj, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, 1 );
}
