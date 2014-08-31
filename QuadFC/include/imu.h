/*
 * imu.h
 *
 *  Created on: 26 mar 2014
 *      Author: martin.lundh
 */

#ifndef IMU_H_
#define IMU_H_


typedef struct imu_data
{
  int16_t accl_x;
  int16_t accl_y;
  int16_t accl_z;
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;
  int16_t barometer;
  int16_t mag_x;
  int16_t mag_y;
  int16_t mag_z;
  int16_t temp;
  uint8_t initialized;
} imu_data_t;


void mpu6050_initialize( );
uint8_t mpu6050_read_motion( imu_data_t *data );

#endif /* IMU_H_ */
