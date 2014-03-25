/*
 * main_control_task.h
 *
 * Created: 2013-03-29 19:07:05
 *  Author: Martin Lundh
 */ 


#ifndef MAIN_CONTROL_TASK_H_
#define MAIN_CONTROL_TASK_H_
/*
 * Fixed point scaling factor.
 */
#define SHIFT_EXP 10

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "asf.h"
#include "debug_macros.h"


/*-----------------------------------------------------------
 *
 * Receiver definitions. All receivers should obey to the format described.
 *
 *-----------------------------------------------------------*/

/*
 * Struct containing the data from an RC receiver.
 */
typedef struct receiver_data {
    int32_t ch0;           //! Channel 0 data.
    int32_t ch1;           //! Channel 1 data.
    int32_t ch2;           //! Channel 2 data.
    int32_t ch3;           //! Channel 3 data.
    int32_t ch4;           //! Channel 4 data.
    int32_t ch5;           //! Channel 5 data.
    int32_t ch6;           //! Channel 6 data.
    int32_t sync;          //! Sync data etc.
    int32_t connection_ok;  //! 0 if ok, != 0 otherwise.
} receiver_data_t;

/*Receiver defenintions*/


/*Receiver queue*/
#define RECEIVER_QUEUE_LENGTH			(2)
#define RECEIVER_QUEUE_ITEM_SIZE        (sizeof(receiver_data_t))
xQueueHandle xQueue_receiver;


/*-----------------------------------------------------------
 * MotorDefenition - All motor implementations should obey 
 * the format described below.
 *-----------------------------------------------------------*/



/*Display queue*/
#define DISPLAY_QUEUE_LENGTH			(1)
#define DISPLAY_QUEUE_ITEM_SIZE         (sizeof(uint8_t *))
#define DISPLAY_STRING_LENGTH_MAX       128
xQueueHandle xQueue_display;


/*Display queue*/
#define DISPLAY_NR_BYTES_QUEUE_LENGTH			(1)
#define DISPLAY_NR_BYTES_QUEUE_ITEM_SIZE         (sizeof(uint8_t))
#define DISPLAY_NR_BYTES_STRING_LENGTH_MAX       128
xQueueHandle xQueue_display_bytes_to_send;

/**/
#define CONFIGURE_REQ_QUEUE_LENGTH			(1)
#define CONFIGURE_REQ_QUEUE_ITEM_SIZE        (sizeof(uint8_t))
xQueueHandle xQueue_configure_req;
/*---------------------------------------------------------*/

/*Crc lookup table.*/
uint16_t  *crcTable;

/*---------------------------------------------------------*/
/*
 * Struct containing the data protocol.
 *
 * frame format:
 * [start][address][control][information][crc][stop]
 */
#define NR_OVERHEAD_BYTES 7
#define MAX_DATA_LENGTH 58
typedef struct communicaion_packet {
    uint8_t address;
    uint8_t control;
    uint8_t length;
	uint8_t *information;
    uint8_t *crc;
    uint8_t *frame;
	uint8_t frame_length;
    } communicaion_packet_t;


/*-----------------------------------------------------------*/
/*
 * IMU definitions. All IMUs should obey to the format described.
 */
 /*-----------------------------------------------------------*/
#define BLOCK_TIME_IMU (2UL / portTICK_RATE_ONE_THIRD_MS)
typedef struct imu_data {
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
}imu_data_t;


/*---------------------------------------------------------*/
/*
 * Struct containing the state information.
 */
typedef struct state_data {
    int32_t pitch;
    int32_t roll;
    int32_t yaw;
    int32_t pitch_rate;
    int32_t roll_rate;
    int32_t yaw_rate;
    int32_t x_pos;
    int32_t y_pos;
    int32_t z_pos;
    int32_t x_vel;
    int32_t y_vel;
    int32_t z_vel;
} state_data_t;
/*---------------------------------------------------------*/
/*
 * Struct containing the control parameters.
 */
typedef struct control_values_pid {
    int32_t roll_p;
    int32_t roll_i;
    int32_t roll_d;
    int32_t pitch_p;
    int32_t pitch_i;
    int32_t pitch_d;
    int32_t yaw_p;
    int32_t yaw_i;
    int32_t yaw_d;
    int32_t altitude_p;
    int32_t altitude_i;
    int32_t altitude_d;    
    } control_values_pid_t;


/*---------------------------------------------------------*/
/*
 * Struct containing the control signal.
 */
typedef struct control_signal {
	int32_t thrust;
	int32_t torque_yaw;
	int32_t torque_pitch;
	int32_t torque_roll;
} control_signal_t;

control_values_pid_t *parameters_rate;
control_values_pid_t *parameters_angle;
state_data_t *state;
state_data_t *setpoint;
control_signal_t *ctrl_signal;
xSemaphoreHandle x_param_mutex;
xSemaphoreHandle x_log_mutex;

/*QuadFC states. The Quadcopter can only be in one of the following staes*/
typedef enum quadfc_state {
	fc_disarmed = 1,
	fc_armed_rate_mode = 2,
	fc_armed_angle_mode = 3,
	fc_config_error = 4,
	fc_initializing = 5,
    fc_configure = 6,
	} quadfc_state_t;

/*each motor can be in one of the following modes*/
typedef enum esc_state {
	esc_off = 1,
	esc_armed_not_running = 2,
	esc_running = 3,
	esc_error = 4,
	}esc_state_t;

typedef enum available_log_param1 {
	time = 1 << 0,
	pitch_ = 1 << 1,
	roll_ = 1 << 2,
	yaw_ = 1 << 3,
	pitch_rate_ = 1 << 4,
	roll_rate_ = 1 << 5,
	yaw_rate_ = 1 << 6,
	x_pos_ = 1 << 7,
	y_pos_ = 1 << 8,
	z_pos_ = 1 << 9,
	x_vel_ = 1 << 10,
	y_vel_ = 1 << 11,
	z_vel_ = 1 << 12,
	ctrl_torque_pitch = 1 << 13,
	ctrl_torque_roll = 1 << 14,
	ctrl_torque_yaw = 1 << 15,
	ctrl_thrust = 1 << 16,
	//ctrl_roll_rate = 1 << 17,
	//ctrl_yaw_rate = 1 << 18,
	//ctrl_x_pos = 1 << 19,
	//ctrl_y_pos = 1 << 20,
	//ctrl_z_pos = 1 << 21,
	//ctrl_x_vel = 1 << 22,
	//ctrl_y_vel = 1 << 23,
	//ctrl_z_vel = 1 << 24,
} available_log_param1_t;

typedef enum available_log_param2 {
	ctrl_error_pitch = 1 << 0,
	ctrl_error_roll = 1 << 1,
	ctrl_error_yaw = 1 << 2,
	ctrl_error_pitch_rate = 1 << 3,
	ctrl_error_roll_rate = 1 << 4,
	ctrl_error_yaw_rate = 1 << 5,
	ctrl_error_x_pos = 1 << 6,
	ctrl_error_y_pos = 1 << 7,
	ctrl_error_z_pos = 1 << 8,
	ctrl_error_x_vel = 1 << 9,
	ctrl_error_y_vel = 1 << 10,
	ctrl_error_z_vel = 1 << 11,
	setpoint_pitch = 1 << 12,
	setpoint_roll = 1 << 13,
	setpoint_yaw = 1 << 14,
	setpoint_pitch_rate = 1 << 15,
	setpoint_roll_rate = 1 << 16,
	setpoint_yaw_rate = 1 << 17,
	setpoint_x_pos = 1 << 18,
	setpoint_y_pos = 1 << 19,
	setpoint_z_pos = 1 << 20,
	setpoint_x_vel = 1 << 21,
	setpoint_y_vel = 1 << 22,
	setpoint_z_vel = 1 << 23,
} available_log_param2_t;

// Global log parameters
int32_t **log_parameters;
uint8_t nr_log_parameters;
uint32_t log_freq;
int32_t time_main;


#define MAX_MOTORS (8)

void create_main_control_task(void);
void main_control_task(void *pvParameters);

void reset_control_error(control_values_pid_t *ctrl_error);
void do_logging(communicaion_packet_t *packet);

#endif /* MAIN_CONTROL_TASK_H_ */
