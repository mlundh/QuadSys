/*
 * common_types.h
 *
 * Copyright (C) 2014 martin
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
#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_
#include "stdint.h"

#define mainDONT_BLOCK                          (0)

typedef struct SpObj SpHandler_t;
typedef struct FlightModeHandler FlightModeHandler_t;
typedef struct CtrlModeHandler CtrlModeHandler_t;

typedef struct eventData eventData_t;

typedef struct eventHandler eventHandler_t;

typedef enum GpioName
{
  ledGreen1,
  ledGreen2,
  ledYellow1,
  ledYellow2,
  ledRed1,
  ledRed2,
  ledNrGpio,
}GpioName_t;

typedef enum CtrlModeState{
  Control_mode_rate,
  Control_mode_attitude,
  Control_mode_not_available,
}CtrlMode_t;

typedef enum mode{
  fmode_init,          // FC is initializing.
  fmode_disarmed,      // Disarmed mode, no action except changing to another state.
  fmode_config,        // Configuration. FC can be configured.
  fmode_arming,        // Motors are arming. No action other than change to error, disarmed or armed state.
  fmode_armed,         // Armed state, FC is flight ready.
  fmode_disarming,     // Disarming.
  fmode_fault,         // FC has encountered a serious problem and has halted.
  fmode_not_available, // state information not available.
}FMode_t;


#define TIME_SHIFT_FACTOR (14)
#define ONE_MS_FP (16)
#define TWO_MS_FP (33)

// Update both macros at the same time!
/*main_control_task execute at 500Hz, period of 2ms*/
#define CTRL_TIME    (2UL / portTICK_PERIOD_MS )
#define CTRL_TIME_FP (TWO_MS_FP)


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
 * up by 14 will correspond to pi rad per second.
 *
 *
 * Thrust:
 * Thrust is not a real state, but is added here for simplicity.
 * Scaling: 16.16 fixed point, 100% = 1<<30

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
#define MAX_U_SIGNAL INT_TO_FIXED(2, FP_16_16_SHIFT)
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
  int8_t acc_sign;
  int8_t gyro_sign;

}ImuOrientation_t;

typedef struct Imu
{
  ImuData_t ImuOffset;
  ImuData_t ImuData;  // Copter coordinate system.
  ImuData_t tempData; // IMU coordinate system
  ImuOrientation_t Orient;
  void *internals;
}Imu_t;



#endif /* COMMON_TYPES_H_ */
