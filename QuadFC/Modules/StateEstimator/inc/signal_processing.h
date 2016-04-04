/*
 * complementary_filter.h
 *
 * Created: 2013-07-08 17:48:55
 *  Author: martin.lundh
 */

#ifndef IMU_SIGNAL_PROCESSING_H_
#define IMU_SIGNAL_PROCESSING_H_
#include "Utilities/inc/common_types.h"

// (int32_t)0.8 * 65536 + 0.5 = 52429
#define FILTER_COEFFICENT_GYRO_FP (52429)

// (int32_t)0.2 * 65536 + 0.5 = 13107
#define FILTER_COEFFICENT_ACCEL_FP (13107)

// (int32_t)0.1 * 65536 + 0.5 = 6554
#define MY (6554)

/**

 */
/**
 * @brief Get euler angles from IMU measurements.
 *
 * Calculations based on info in Freescale Semiconductor Application Note
 * Document Number: AN3461
 * Rev. 6, 03/2013
 *
 * Convention used in the aerospace sequence is that the roll angle can range between -pi rad to +pi rad but the
 * pitch angle is restricted to -pi rad to +pi rad. This is used here.
 * @param state         The function updates the appropriate states in the state data.
 * @param measurments   IMU data.
 */
void Signal_getEulerAnglesAccel( state_data_t *state, ImuData_t *measurments );

/**
 * @brief Get euler angles from gyro measurements.
 *
 * The function uses the time step to integrate the angle from measurements from angular
 * velocity. This means that it will drift.
 * @param state         The function updates the appropriate states in the state data.
 * @param measurments   IMU data.
 */
void Signal_getEulerAnglesGyro( state_data_t *state, ImuData_t *measurments, control_time_t* time);

/**
 * @brief Update the state vector with current rotational velocities based on imu
 * readings.
 * @param state         The function updates the appropriate states in the state data.
 * @param measurments   IMU data.
 */
void Signal_getRateGyro( state_data_t *state, ImuData_t *measurments );

/**
 * @brief Combine angular measurements from two sources.
 *
 * Trust gyro for short term measurements, and accelerometer for
 * long term behavior.
 * @param state_accel
 * @param state_gyro
 * @param state
 */
void Signal_complemetaryFilter( state_data_t *state_accel, state_data_t *state_gyro, state_data_t *state );
#endif /* IMU_SIGNAL_PROCESSING_H_ */

