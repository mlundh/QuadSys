/*
 * complementary_filter.h
 *
 * Created: 2013-07-08 17:48:55
 *  Author: martin.lundh
 */ 


#ifndef IMU_SIGNAL_PROCESSING_H_
#define IMU_SIGNAL_PROCESSING_H_
#include "my_math.h"
#include "main_control_task.h"

/*
*   IMU scaling factors computed. (max angle)/(max value) = angle/unit
* 
*/
#define MPU6050_RAD_PER_SEC_LSB_500 0,0002663 //(500 deg/s in rad/s)/(2^15) = 0,0002663 rad/s/unit
/* 
 * Receiver scaling factors computed. receiver has a 12 bit format and we want that to
 * correspond to 180 deg/s
 */
#define RECEIVER_LSB_ANGLE_60DEG 0,0005113 // (60 deg/s in rad/s)/(2^11) = 0,0005113 rad/unit
#define RECEIVER_RAD_PER_SEC_LSB_180 0,001534 //(180 deg/s in rad/s)/(2^11) = 0,001534 rad/s/unit

/* Since we want to keep accuracy we are dividing the smaller number by the bigger. */

/* if converting IMU values to receiver units then: 
 * ((180 deg/s in rad/s)/(2^11)) / ((500 deg/s in rad/s)/(2^15)) = 5,76 -> IMU*5,76 = receiver
 */
#define CONVERT_IMU_TO_RECEIVER_UNITS 1 
#define CONVERT_RECEIVER_TO_IMU_UINTS 6 // receiver * (IMU*5,76/receiver) = IMU
 


#define TIME_STEP 0 // 0.002s * MULT_FACTOR
#define FILTER_COEFFICENT_GYRO 8
#define FILTER_COEFFICENT_ACCEL 2
#define MY 0


void get_euler_angles_accel(state_data_t *state, imu_data_t *measurments);
void get_euler_angles_gyro(state_data_t *state, imu_data_t *measurments);
void get_rate_gyro(state_data_t *state, imu_data_t *measurments);
void complemetary_filter(state_data_t *state_accel, state_data_t *state_gyro, state_data_t *state);
void translate_receiver_signal_angle(state_data_t *setpoint, receiver_data_t *received_data);
void translate_receiver_signal_rate(state_data_t *setpoint, receiver_data_t *received_data);
#endif /* IMU_SIGNAL_PROCESSING_H_ */

