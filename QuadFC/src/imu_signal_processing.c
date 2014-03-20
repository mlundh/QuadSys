/*
 * complementary_filter.c
 *
 * Copyright (C) 2014  Martin Lundh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "imu_signal_processing.h"
#include "mpu6050.h"
#include "my_math.h"



void get_euler_angles_accel(state_data_t *state, imu_data_t *measurments)
{
    // TODO DO NOT USE UNTILL FIXED. MY_ATAN DOES NOT WORK NOW!
	int32_t numerator = 0;
	int32_t denominator = 0;
	
	
	// pitch calculation
	numerator = (int32_t)(measurments->accl_y);
	denominator = (my_sgn(measurments->accl_z)) * (int32_t)(my_square_root((my_square(measurments->accl_z) + my_mult(my_square(measurments->accl_x),MY))));

	state->pitch = my_atan2(numerator,denominator);
	// roll calculation
	numerator = (int32_t)(measurments->accl_x);
	denominator = (int32_t)(my_square_root(my_square(measurments->accl_x) + my_square(measurments->accl_z)));
	
	state->roll = my_atan2(numerator, denominator);
}

void get_euler_angles_gyro(state_data_t *state, imu_data_t *measurments)
{
    /*
    * TODO
    * DO NOT USE UNTIL FIXED
    */
	state->pitch += my_mult(my_mult(((int32_t)measurments->gyro_x), (int32_t)CONVERT_IMU_TO_RECEIVER_UNITS), TIME_STEP);
	state->roll += my_mult(my_mult(((int32_t)measurments->gyro_y), (int32_t)CONVERT_IMU_TO_RECEIVER_UNITS), TIME_STEP);
	state->yaw += my_mult(my_mult(((int32_t)measurments->gyro_z), (int32_t)CONVERT_IMU_TO_RECEIVER_UNITS), TIME_STEP);
}

void get_rate_gyro(state_data_t *state, imu_data_t *measurments)
{
    /* Gyro rates are scaled to match receiver units. 
     */
	state->pitch_rate = ((int32_t)measurments->gyro_x) * (int32_t)CONVERT_IMU_TO_RECEIVER_UNITS;
	state->roll_rate = ((int32_t)measurments->gyro_y) * (int32_t)CONVERT_IMU_TO_RECEIVER_UNITS;
	state->yaw_rate = ((int32_t)measurments->gyro_z) * (int32_t)CONVERT_IMU_TO_RECEIVER_UNITS;
}

void complemetary_filter(state_data_t *state_accel, state_data_t *state_gyro, state_data_t *state)
{
	state->pitch = my_mult((int32_t)state_gyro->pitch,(int32_t)FILTER_COEFFICENT_GYRO) + my_mult((int32_t)state_accel->pitch, (int32_t)FILTER_COEFFICENT_ACCEL);
	state->roll = my_mult((int32_t)state_gyro->roll,(int32_t)FILTER_COEFFICENT_GYRO) + my_mult((int32_t)state_accel->roll, (int32_t)FILTER_COEFFICENT_ACCEL);
	/*Update gyro state to reduce accumulatory errors.*/
	state_gyro->pitch = state->pitch;
	state_gyro->roll = state->roll;
}


/*Translates the received signal into fixed point used in project
 * Receiver scaled to match IMU
 *
 *
 */ 

void translate_receiver_signal_angle(state_data_t *setpoint, receiver_data_t *received_data)
{
    setpoint->z_vel = ((received_data->ch0));
    setpoint->roll = ((received_data->ch1 - SATELLITE_CH_CENTER) * (int32_t)CONVERT_RECEIVER_TO_IMU_UINTS);
    setpoint->pitch = ((received_data->ch2 - SATELLITE_CH_CENTER) * (int32_t)CONVERT_RECEIVER_TO_IMU_UINTS);
    setpoint->yaw_rate = ((received_data->ch3 - SATELLITE_CH_CENTER) * (int32_t)CONVERT_RECEIVER_TO_IMU_UINTS);
}
/*Translates the received signal into fixed point used in project*/
void translate_receiver_signal_rate(state_data_t *setpoint, receiver_data_t *received_data)
{
    setpoint->z_vel = (received_data->ch0);
    setpoint->roll_rate = -((received_data->ch1 - SATELLITE_CH_CENTER) * (int32_t)CONVERT_RECEIVER_TO_IMU_UINTS) << 2;
    setpoint->pitch_rate = -((received_data->ch2 - SATELLITE_CH_CENTER) * (int32_t)CONVERT_RECEIVER_TO_IMU_UINTS) << 2;
    setpoint->yaw_rate = -((received_data->ch3 - SATELLITE_CH_CENTER) * (int32_t)CONVERT_RECEIVER_TO_IMU_UINTS) << 2;
    
}
