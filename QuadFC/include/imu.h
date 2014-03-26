/*
 * imu.h
 *
 *  Created on: 26 mar 2014
 *      Author: martin.lundh
 */

#ifndef IMU_H_
#define IMU_H_
#include "main_control_task.h"

void mpu6050_initialize();
uint8_t mpu6050_read_motion(imu_data_t *data);

#endif /* IMU_H_ */
