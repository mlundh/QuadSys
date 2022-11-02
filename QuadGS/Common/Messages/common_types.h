/*
 * common_types.h
 *
 * Copyright (C) 2019 Martin Lundh
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

#ifndef QUADGS_COMMON_TYPES_H_
#define QUADGS_COMMON_TYPES_H_
#include <stdint.h>
namespace QuadGS {


#define TIME_SHIFT_FACTOR (14)
#define ONE_MS_FP (16)
#define TWO_MS_FP (33)

// Update both macros at the same time!
/*main_control_task execute at 1000Hz, period of 1ms*/
#define CTRL_TIME    (1UL / portTICK_PERIOD_MS )
#define CTRL_TIME_FP (ONE_MS_FP)
#define CTRL_FREQ    ((1 / CTRL_TIME) * ( TickType_t )1000) // CTRL_TIME is expressed in ms therefore we multiply by 1000

/**
 * @brief Time scaling used in the project.
 *
 * Most time constants will be expressed in milliseconds, we should have
 * enough precision to express milliseconds.
 * 1/(2^(14)) = 0.00006103515sec/LSB.
 *
 */
typedef struct Control_time
{
  uint32_t value;   //!< 0.000003815sec/LSB.
}control_time_t;

#define FP_16_16_SHIFT (16)
#define MAX16f (65536.0)
// y is the shift factor.
#define INT_TO_FIXED(x,y)         ((x) << (y))
// y is max number of the integer part of the fṕ.
#define DOUBLE_TO_FIXED(x, y)     ((int32_t)(x * y + 0.5))
// y is the shift factor
#define FIXED_TO_INT(x,y)         ((x) >> (y))
// y is the max number of the integer part of the fp.
#define FIXED_TO_DOUBLE(x, y)     (((double)x) / (y))

/**
 * @brief state vector describing the copters position and attitude in
 * the inertial reference frame (earth frame).
 * Angle and angular speed are expressed in 16.16 signed fixed point
 * with scaling so that 1<<16 = pi.
 *
 * The state vector is described in SI units with a scale factor.
 *
 * Scale factors used:
 * Angle:
 * pi rad/(2^(16)) = 0.0000479368997 rad/LSB, gives that 1 LSB shifted
 * up by 16 will correspond to pi rad. This is BRAND_PI.
 *
 * Angular velocity:
 * pi rad/(2^(16)) = 0.0000479368997 rad/s/LSB, gives that 1 LSB
 * shifted up by 16 will correspond to pi rad per second.
 *
 *
 * Position:
 * TODO
 *
 * Linear velocity:
 * TODO
 */
typedef enum state_names_inertial_frame
{
  x_angle_if = 0,         //!< 0.0000479368997 rad/LSB
  y_angle_if = 1,         //!< 0.0000479368997 rad/LSB
  z_angle_if = 2,         //!< 0.0000479368997 rad/LSB
  x_anglVel_if = 3,       //!< 0.0000479368997 rad/s/LSB
  y_anglVel_if = 4,       //!< 0.0000479368997 rad/s/LSB
  z_anglVel_if = 5,       //!< 0.0000479368997 rad/s/LSB
  x_pos_if = 6,           //!< 0.0000479368997 rad/s/LSB
  y_pos_if = 7,           //!<
  z_pos_if = 8,           //!<
  x_vel_if = 9,           //!<
  y_vel_if = 10,          //!<
  z_vel_if = 11,          //!<
  nr_states_if = 12,      //!<
}state_names_if_t;


/**
 * @brief state vector describing the copters attitude in
 * the body reference frame.
 *
 * In order to get the orientation of the copter in terms of angles,
 * the body frame is linked to the inertial frame by euler angles.
 * The euler angles are denoted pitch, roll and yaw. This is common
 * practice in avionics.
 *
 * The state vector is described in SI units with a scale factor.
 *
 * Scale factors used:
 * angle: 16.16 signed fixed point with 1 as pi rad.
 * pi rad/(2^(16)) = 0.0000479368997 rad/LSB, gives that 1 LSB shifted
 * up by 16 will correspond to pi rad. This is a BRAND.
 *
 * Angular velocity: 16.16 signed fixed point with 1 as pi rad / second.
 * pi rad/(2^(16)) = 0.0000479368997 rad/s/LSB, gives that 1 LSB shifted
 * up by 16 will correspond to pi rad per second.
 *
 *
 * Thrust:
 * Thrust is not a real state, but is added here for simplicity.
 * Scaling: 16.16 fixed point, 100% = 1<<16
 * 
 * 
 * The system uses a right-hand coordinate system with:
 * X pointing forward.
 * Y pointing to the left.
 * Z pointing up.
 * 
 * This means that:
 * Yaw is a rotation around Z in the counter-clockwise direction viewd from above
 * Pitch is a rotation around Y leaning forward.
 * Roll is a rotation around X to the right.


 */

typedef enum state_names_body_frame
{
  pitch_bf = 0,           //!< 0.0000479368997 rad/LSB
  roll_bf = 1,            //!< 0.0000479368997 rad/LSB
  yaw_bf = 2,             //!< 0.0000479368997 rad/LSB
  pitch_rate_bf = 3,      //!< 0.0000479368997 rad/s/LSB
  roll_rate_bf = 4,       //!< 0.0000479368997 rad/s/LSB
  yaw_rate_bf = 5,        //!< 0.0000479368997 rad/s/LSB
  thrust_sp = 6,          //!< Thrust added for setpoint reasons.
  nr_states_bf = 7,       //!<

}state_names_bf_t;

typedef struct state_data
{
  int32_t state_bf[nr_states_bf];  //!< body frame state vector.
  uint16_t state_valid_bf;         //!< bits used to determine if the state is valid.
  int32_t state_if[nr_states_if];  //!< Inertial frame state vector.
  uint16_t state_valid_if;         //!< bits used to determine if the state is valid.
}state_data_t;


/**
 * Control signal struct. Values in 16.16 fixed point.
 * 100% = 1<<16. Resolution = 0,0000015259
 */
#define MAX_U_SIGNAL INT_TO_FIXED(1, FP_16_16_SHIFT)
typedef enum control_signal_names
{
  u_pitch = 0,
  u_roll = 1,
  u_yaw = 2,
  u_thrust = 3,
}control_signal_names_t;

typedef struct control_signal
{
  int32_t control_signal[4];
}control_signal_t;



/**
 * @brief Vector describing the data retreived from an IMU.
 *
 * Scale factors used:
 * accelerometer:
 * 16.16 fixedpoint with 1 as 1g.
 *
 *
 * gyro:
 * Angular velocity: 16.16 fixed point with 1 as pi rad per sec.
 *
 *
 * magnetometer:
 * todo
 *
 * thermometer
 * TODO
 */
typedef enum ImuNames
{
  accl_x = 0,             //!< 0.00001525879 g/LSB
  accl_y = 1,             //!<
  accl_z = 2,             //!<
  gyro_x = 3,             //!< 0.0000479368997 rad/s/LSB
  gyro_y = 4,             //!< 0.0000479368997 rad/s/LSB
  gyro_z = 5,             //!< 0.0000479368997 rad/s/LSB
  mag_x = 6,              //!<
  mag_y = 7,              //!<
  mag_z = 8,              //!<
  temp_imu = 9,           //!<
  nr_imu_values = 10,     //!<
}ImuNames_t;

typedef struct ImuData
{
  int32_t imu_data[nr_imu_values];
  uint8_t confidence[nr_imu_values];
}ImuData_t;

typedef struct ImuOrientation
{
  int32_t r_0_0;
  int32_t r_0_1;
  int32_t r_0_2;
  int32_t r_1_0;
  int32_t r_1_1;
  int32_t r_1_2;
  int32_t r_2_0;
  int32_t r_2_1;
  int32_t r_2_2;

}ImuOrientation_t;

/**
 * Stream operator ImuData_t struct.
 */
inline BinaryIStream&
operator>>(BinaryIStream& stream, state_data_t& state)
{
	for(int i = 0; i < nr_states_bf; i++)
	{
		stream >> state.state_bf[i];
	}
	stream >> state.state_valid_bf;
	for(int i = 0; i < nr_states_if; i++)
	{
		stream >> state.state_if[i];
	}
	stream >> state.state_valid_if;
	return stream;
}
/**
 * Stream operator ImuData_t struct.
 */
inline BinaryOStream&
operator<<(BinaryOStream& stream, const state_data_t& state)
{
	for(int i = 0; i < nr_states_bf; i++)
	{
		stream << state.state_bf[i];
	}
	stream << state.state_valid_bf;
	for(int i = 0; i < nr_states_if; i++)
	{
		stream << state.state_if[i];
	}
	stream << state.state_valid_if;
	return stream;
}

/**
 * Stream operator ImuData_t struct.
 */
inline BinaryIStream&
operator>>(BinaryIStream& stream, control_signal_t& ctrl)
{
	for(int i = 0; i < nr_imu_values; i++)
	{
		stream >> ctrl.control_signal[i];
	}
	return stream;
}


/**
 * Stream operator ImuData_t struct.
 */
inline BinaryOStream&
operator<<(BinaryOStream& stream, const control_signal_t& ctrl)
{
	for(int i = 0; i < nr_imu_values; i++)
	{
		stream << ctrl.control_signal[i];
	}
	return stream;
}
/**
 * Stream operator ImuData_t struct.
 */
inline BinaryIStream&
operator>>(BinaryIStream& stream, ImuData_t& imu)
{
	for(int i = 0; i < nr_imu_values; i++)
	{
		stream >> imu.imu_data[i];
	}
	for(int i = 0; i < nr_imu_values; i++)
	{
		stream >> imu.confidence[i];
	}
	return stream;
}


/**
 * Stream operator ImuData_t struct.
 */
inline BinaryOStream&
operator<<(BinaryOStream& stream, const ImuData_t& imu)
{
	for(int i = 0; i < nr_imu_values; i++)
	{
		stream << imu.imu_data[i];
	}
	for(int i = 0; i < nr_imu_values; i++)
	{
		stream << imu.confidence[i];
	}
	return stream;
}

/**
 * Stream operator ImuOrientation struct.
 */
inline BinaryIStream&
operator>>(BinaryIStream& stream, ImuOrientation_t& imu)
{
	stream >> imu.r_0_0;
	stream >> imu.r_0_1;
	stream >> imu.r_0_2;
	stream >> imu.r_1_0;
	stream >> imu.r_1_1;
	stream >> imu.r_1_2;
	stream >> imu.r_2_0;
	stream >> imu.r_2_1;
	stream >> imu.r_2_2;
	stream >> imu.acc_sign;
	stream >> imu.gyro_sign;

	return stream;
}

/**
 * Stream operator ImuOrientation struct.
 */
inline BinaryOStream&
operator<<(BinaryOStream& stream, const ImuOrientation_t& imu)
{
	stream << imu.r_0_0;
	stream << imu.r_0_1;
	stream << imu.r_0_2;
	stream << imu.r_1_0;
	stream << imu.r_1_1;
	stream << imu.r_1_2;
	stream << imu.r_2_0;
	stream << imu.r_2_1;
	stream << imu.r_2_2;
	stream << imu.acc_sign;
	stream << imu.gyro_sign;

	return stream;
}


}
#endif /* QUADGS_COMMON_TYPES_H_ */

