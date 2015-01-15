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

#include "mpu6050.h"
#include "led_control_task.h"
#include "globals.h"

static imu_data_t offset = { /*IMU data struct, used to store the GYRO offset values*/
.accl_x = 0,
    .accl_y = 0,
    .accl_z = 0,
    .gyro_x = 0,
    .gyro_y = 0,
    .gyro_z = 0,
    .mag_x = 0,
    .mag_y = 0,
    .mag_z = 0,
    .temp = 0,
    .barometer = 0
};

static mpu6050_data_t mpu6050_packet;

// dev_addr Device address
// reg_addr Register address
// bit_nr First bit to read (0-7)
// data Data to be written to previously defined bits
uint8_t mpu6050_write_settings( uint8_t reg_addr, uint8_t bit_nr, uint8_t nr_bits, uint8_t data, uint8_t nr_bytes )
{

  status_code_t twi_status = 0;
  uint8_t current_reg_value = 0;

  mpu6050_read_byte( reg_addr, &current_reg_value );

  uint8_t mask = ((1 << nr_bits) - 1) << (bit_nr - nr_bits + 1); // create mask.

  data <<= (bit_nr - nr_bits + 1);        // shift data into correct position
  data &= mask;                           // zero all non-important bits in data
  current_reg_value &= ~(mask);           // zero all important bits in existing word
  current_reg_value |= data;              // combine data with existing word

  data = (current_reg_value & 0xFF);

  mpu6050_packet.twi_data.addr[0] = reg_addr;                   // TWI slave memory address data
  *mpu6050_packet.data = data;
  mpu6050_packet.twi_data.length = 1;			               // transfer data size (bytes)

  xSemaphoreTake( mpu6050_packet.twi_notification_semaphore, mpu6050_packet.xtransmit_block_time );

  twi_status = freertos_twi_write_packet_async( mpu6050_packet.twi,
      &mpu6050_packet.twi_data,
      mpu6050_packet.xtransmit_block_time,
      mpu6050_packet.twi_notification_semaphore );
  //Wait until the data is written.
  if ( (xSemaphoreTake(mpu6050_packet.twi_notification_semaphore, mpu6050_packet.xtransmit_block_time) == pdPASS) && (twi_status == STATUS_OK))
  {
    xSemaphoreGive( mpu6050_packet.twi_notification_semaphore );
  }
  else
  {
    uint8_t led_state = error_TWI_led;
    xQueueSendToBack( xQueue_led, &led_state, mainDONT_BLOCK );
  }
  return 0;
}

uint8_t mpu6050_read_settings( uint8_t reg_addr, uint8_t bit_nr, uint8_t nr_bits, uint8_t *data, uint8_t nr_bytes )
{

  status_code_t twi_status = 0;
  mpu6050_packet.twi_data.addr[0] = reg_addr;                   // TWI slave memory address data
  mpu6050_packet.twi_data.length = nr_bytes;			        // transfer data size (bytes)

  xSemaphoreTake( mpu6050_packet.twi_notification_semaphore, mpu6050_packet.xtransmit_block_time );

  twi_status = freertos_twi_read_packet_async( mpu6050_packet.twi,
      &mpu6050_packet.twi_data,
      mpu6050_packet.xtransmit_block_time,
      mpu6050_packet.twi_notification_semaphore );
  //Wait on completion.
  if (( xSemaphoreTake(mpu6050_packet.twi_notification_semaphore, mpu6050_packet.xtransmit_block_time) == pdPASS) && (twi_status == STATUS_OK))
  {
    xSemaphoreGive( mpu6050_packet.twi_notification_semaphore );
  }
  else
  {
    uint8_t led_state = error_TWI_led;
    xQueueSendToBack( xQueue_led, &led_state, mainDONT_BLOCK );
  }

  uint8_t w = *mpu6050_packet.data;
  uint8_t mask = ((1 << nr_bits) - 1) << (bit_nr - nr_bits + 1);
  w &= mask;
  w >>= (bit_nr - nr_bits + 1);
  *data = w;

  return 0;
}

uint8_t mpu6050_read_motion( imu_data_t *data )
{
  status_code_t twi_status = 0;
  mpu6050_packet.twi_data.addr[0] = MPU6050_RA_ACCEL_XOUT_H;                   // TWI slave memory address data
  mpu6050_packet.twi_data.length = 14;			        // transfer data size (bytes)

  if ( xSemaphoreTake(mpu6050_packet.twi_notification_semaphore, mpu6050_packet.xtransmit_block_time) == pdPASS )
  {
    twi_status = freertos_twi_read_packet_async( mpu6050_packet.twi,
        &mpu6050_packet.twi_data,
        mpu6050_packet.xtransmit_block_time,
        mpu6050_packet.twi_notification_semaphore );

    //Wait on completion
    if (( xSemaphoreTake(mpu6050_packet.twi_notification_semaphore,mpu6050_packet.xtransmit_block_time) == pdPASS) && (twi_status == STATUS_OK))
    {
      xSemaphoreGive( mpu6050_packet.twi_notification_semaphore );
    }
    else
    {
      uint8_t led_state = error_TWI_led;
      xQueueSendToBack( xQueue_led, &led_state, mainDONT_BLOCK );
    }
    //TODO check validity of data!

    data->accl_x = (int16_t) (mpu6050_packet.data[0] << 8 | mpu6050_packet.data[1]);
    data->accl_y = (int16_t) (mpu6050_packet.data[2] << 8 | mpu6050_packet.data[3]);
    data->accl_z = (int16_t) (mpu6050_packet.data[4] << 8 | mpu6050_packet.data[5]);
    data->temp = (int16_t) (mpu6050_packet.data[6] << 8 | mpu6050_packet.data[7]);
    data->gyro_x = -((int16_t) (mpu6050_packet.data[8] << 8 | mpu6050_packet.data[9])) - offset.gyro_x;
    data->gyro_y = ((int16_t) (mpu6050_packet.data[10] << 8 | mpu6050_packet.data[11])) - offset.gyro_y;
    data->gyro_z = -((int16_t) (mpu6050_packet.data[12] << 8 | mpu6050_packet.data[13])) - offset.gyro_z;
    return 0;
  }
  else
  {
    //Error!
  }
  return 1;
}

void mpu6050_calc_offset( )
{
  const TickType_t xPeriod = (10UL / portTICK_PERIOD_MS); /*Read new values at 100Hz*/

  imu_data_t imu_readings;
  int i = 0;
  /*The first readings might be nonsense*/
  for ( i = 0; i < 100; i++ )
  {
    mpu6050_read_motion( &imu_readings );
    vTaskDelay( xPeriod );
  }

  int64_t temp_accl_x = 0;
  int64_t temp_accl_y = 0;
  int64_t temp_accl_z = 0;
  int64_t temp_gyro_x = 0;
  int64_t temp_gyro_y = 0;
  int64_t temp_gyro_z = 0;

  for ( i = 0; i < 2000; i++ )
  {
    mpu6050_read_motion( &imu_readings );
    temp_accl_x += (int64_t) imu_readings.accl_x;
    temp_accl_y += (int64_t) imu_readings.accl_y;
    temp_accl_z += (int64_t) imu_readings.accl_z;
    temp_gyro_x += (int64_t) imu_readings.gyro_x;
    temp_gyro_y += (int64_t) imu_readings.gyro_y;
    temp_gyro_z += (int64_t) imu_readings.gyro_z;
    // TODO check validity of measurments!!
    vTaskDelay( xPeriod );
  }

  offset.accl_x = (int16_t) (temp_accl_x / 2000);
  offset.accl_y = (int16_t) (temp_accl_y / 2000);
  offset.accl_z = (int16_t) (temp_accl_z / 2000);
  offset.gyro_x = (int16_t) (temp_gyro_x / 2000);
  offset.gyro_y = (int16_t) (temp_gyro_y / 2000);
  offset.gyro_z = (int16_t) (temp_gyro_z / 2000);
}

void mpu6050_initialize( )
{
  static TickType_t delay_1000_ms = (1000UL / portTICK_PERIOD_MS); /*delay of 1000ms*/

  mpu6050_packet.twi = TWI0;
  mpu6050_packet.twi_notification_semaphore = twi_0_notification_semaphore;
  mpu6050_packet.xtransmit_block_time = BLOCK_TIME_IMU;
  mpu6050_packet.twi_data.chip = MPU6050_ADDRESS_DORTEK;
  mpu6050_packet.twi_data.addr_length = 1;
  mpu6050_packet.twi_data.buffer = mpu6050_packet.data;

  mpu6050_reset();
  vTaskDelay( delay_1000_ms );

  mpu6050_setClockSource( MPU6050_CLOCK_PLL_XGYRO );
  mpu6050_setFullScaleGyroRange( MPU6050_GYRO_FS_500 );
  mpu6050_setFullScaleAccelRange( MPU6050_ACCEL_FS_16 );
  mpu6050_setSleepEnabled( 0 );
  mpu6050_setDLPFMode( MPU6050_DLPF_BW_42 );

  vTaskDelay( delay_1000_ms );

  /*Calculate offsets for gyro. TODO calculate angle offset for accl.*/
  mpu6050_calc_offset();

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
void mpu6050_setClockSource( uint8_t source )
{
  mpu6050_write_settings( MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source, 1 );
}

/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
void mpu6050_setFullScaleGyroRange( uint8_t range )
{
  mpu6050_write_settings( MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range, 1 );
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
void mpu6050_getFullScaleAccelRange( uint8_t *buffer )
{
  mpu6050_read_settings( MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, buffer, 1 );

}

/** Set full-scale accelerometer range.
 * @param range New full-scale accelerometer range setting
 * @see getFullScaleAccelRange()
 */
void mpu6050_setFullScaleAccelRange( uint8_t range )
{
  mpu6050_write_settings( MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range, 1 );
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
void mpu6050_getSleepEnabled( uint8_t *buffer )
{
  mpu6050_read_bit( MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, buffer );
}
/** Set sleep mode status.
 * @param enabled New sleep mode enabled status
 * @see getSleepEnabled()
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_SLEEP_BIT
 */
void mpu6050_setSleepEnabled( uint8_t enabled )
{
  mpu6050_write_bit( MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled );
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
uint8_t mpu6050_getDLPFMode( uint8_t *buffer )
{
  mpu6050_read_settings( MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, buffer, 1 );
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
void mpu6050_setDLPFMode( uint8_t mode )
{
  mpu6050_write_settings( MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode, 1 );
}

// PWR_MGMT_1 register

/** Trigger a full device reset.
 * A small delay of ~50ms may be desirable after triggering a reset.
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_DEVICE_RESET_BIT
 */
void mpu6050_reset( )
{
  mpu6050_write_bit( MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, 1 );
}
