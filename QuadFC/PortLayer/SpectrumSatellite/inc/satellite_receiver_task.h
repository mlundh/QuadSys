/*
 * receiver_task.h
 *
 * Created: 2013-03-24 14:51:05
 *  Author: Martin Lundh
 */

#ifndef SATELLITE_RECEIVER_TASK_H_
#define SATELLITE_RECEIVER_TASK_H_

/**
 * @brief Handles a satellite receiver.
 *
 * The module that handles a satellite receiver. It will first build confidence in the
 * configuration since multiple configurations are supported. The transmitter-receiver
 * combination determines the frame configuration.
 *
 */




#include "FreeRTOS.h"
#include "task.h"
#include "Modules/MsgBase/inc/common_types.h"
#include "Modules/Messages/inc/Msg_FlightModeReq.h"
#include "Modules/Messages/inc/Msg_CtrlModeReq.h"
#include "Modules/Messages/inc/Msg_NewSetpoint.h"
#include "Modules/EventHandler/inc/event_handler.h"
#include "Components/Parameters/inc/paramHandler.h"

/// Defines used by the module.
#define SATELLITE_DATA_MASK 0x7ff            /*Data contained in the 11 least significant bits*/
#define SATELLITE_CHANNEL_MASK 0x7800        /*Channel number contained in the 4 bits above data.*/

#define BAUD_SATELLITE 115200
#define SATELITE_USART (0)

#define SATELLITE_MESSAGE_LENGTH 16

#define SATELLITE_CH_CENTER 1022

#define CHANNEL_CONFIDENCE (1)
#define FRAME_CONFIDENCE_MAX (15)
#define FRAME_CONFIDENCE_THRESHOLD (10)
#define NUMBER_OF_CHANNELS (15)
#define MAX_NUMBER_OF_FRAME_TYPES (3)

#define MESSAGE_VALID_FOR_MS (50) // A setpoint generated by this module is valid for 50ms.
/**
 * Resolution of receiver:
 * 4pi Rad/s/(2^(11)) = 0,0061359
 *
 * Resolution for state vector rate:
 * pi rad/(2^(16)) = 0.0000479368997 rad/s resolution.
 *
 * (receiver resolution)/(state resolution) = 128
 *
 * This is the quote used to scale between receiver
 * and state. The state uses a 16.16 fixed point,
 * converting the quote to this format:
 * (int32_t)128 * 65536 + 0.5 = 8388608
 * This leads to the following define:
 *
 */
#define SPECTRUM_TO_STATE_RATE (8388608)

/**
 * Resolution of receiver:
 * 2pi Rad/(2^(11)) = 0,00306796
 *
 * Resolution for state vector rate:
 * pi rad/(2^(16)) = 0.0000479368997 rad resolution.
 *
 * (receiver resolution)/(state resolution) = 64
 *
 * This is the quote used to scale between receiver
 * and state. The state uses a 16.16 fixed point,
 * converting the quote to this format:
 * (int32_t)128 * 65536 + 0.5 = 4194304
 * This leads to the following define:
 *
 */
#define SPECTRUM_TO_STATE_ANGLE (4194304)

/**
 * max receiver thrust: 100% = 7FF
 *
 * max control signal:    100% = 1<<16 = 65536
 *
 * (max control signal)/(max receiver thrust) = 32
 *
 * This is the quote used to scale between receiver
 * and state for throttle.
 * This leads to the following define:
 */
#define SPECTRUM_TO_CTRL_THROTTLE (32)

/**
 * Struct describing the value of the specific channel.
 */
typedef struct spektrum_channel {
  uint16_t value;
} spektrum_channel_t;

/**
 * Struct describing one frame type. There might be multiple frame types needed
 * to transmit all data from the satellite receiver. Each frame carries a fixed set
 * of channels.
 */
typedef struct spektrum_frame {
  uint16_t channels; // Used as bit-field indexed by channel no.
  uint8_t confidence;
} spektrum_frame_config_t;

/**
 * Struct describing the current frame configuration. First the module will build
 * confidence in the setup, and only when a confident frame configuration is preset
 * will the module send setpoint data to the setpoint handler.
 */
typedef struct spectrum_config {
  spektrum_frame_config_t frame_types_present[MAX_NUMBER_OF_FRAME_TYPES];
  uint8_t confident_frame_config;
  uint16_t channels_available; // Used as bit-field indexed by channel no. Contains all channels in all frames.
}spectrum_config_t;


/**
 * Struct describing the current data
 */
typedef struct spektrum_data {
  spektrum_channel_t ch[NUMBER_OF_CHANNELS];
  uint16_t channels_merged;     // Used as bit-field indexed by channel no. Contains all the merged channels.
  uint16_t channels_last_read; // Used as bit-field indexed by channel no. Contains the channels in the last frame.
} spektrum_data_t;

/**
 * Struct containing all the data the "object" needs.
 */
typedef struct Satellite
{
  spektrum_data_t* decoded_data;
  spectrum_config_t* configuration;
  state_data_t* setpoint;
  uint8_t *satellite_receive_buffer;
  int32_t multiplier;
  int32_t divisor;
  int32_t throMult;
  FlightMode_t current_flight_mode_state;
  CtrlMode_t current_control_mode;
  eventHandler_t* evHandler;
  paramHander_t* paramHandler;
}Satellite_t;

/**
 * Initialize the satellite module. Allocates all data fields needed by the task.
 * @param stateHandler
 * @param setpointHandler
 * @return Null if fail, pointer to a struct instance otherwise.
 */
Satellite_t* Satellite_Init(eventHandler_t* eventMaster);

/**
 * @brief The receiver task. Reads serial data and decodes it. Then transmits the decoded setpoint to
 * the setpoint handler.
 *
 * It is necessary to sync communication with the satellite receiver to
 * ensure that each frame is captured as a whole. A frame consists of
 * 16 consecutive bytes that should be received without delay. A new
 * frame should arrive every 11 or 22 ms.
 *
 * @param pvParameters  Task parameters.
 */
void Satellite_ReceiverTask( void *pvParameters );

/**
 * Decode raw data from the serial port.
 * @param data              Raw data from the port.
 * @param configuration     A handle to the current configuration.
 * @param spectrum_data     Output data.
 * @return
 */
int Satellite_DecodeSignal(uint8_t* data, spectrum_config_t* configuration, spektrum_data_t* spectrum_data);

/**
 * Lock the valid frame types and number of channels needed for a valid setpoint. This function
 * should be called a number of times with decoded data (it only needs the bitmask of the
 * received channels) to lock a frame setup.
 * @param configuration                 Pointer to a configuration struct.
 * @param channels_in_current_frame     each bit represents one bit in the frame that is to be registered.
 * @return
 */
uint8_t Satellite_LockFrameType(spectrum_config_t* configuration, uint16_t channels_in_current_frame);

/**
 * Map receiver channels to setpoint.
 *
 * Scaling of state for rate is 1pi rad/s = 1<<16. Max (default) rate from receiver is 2pi rad/s.
 * Scaling of state for angle is 1pi rad = 1<<16. Max (default) rate from receiver is pi rad.
 *
 * @param obj               The instance object.
 * @param reciever_data     The valid and complete frames.
 * @param setpoint          Output, setpoint.
 */
void Satellite_MapToSetpoint(Satellite_t* obj, spektrum_data_t *reciever_data, state_data_t *setpoint);


/**
 * The receiver can update the FC internal state (change to dissarmed etc.).
 * @param obj               The instance object.
 * @param merged_data       The valid and complete frames.
 * @return
 */
uint8_t Satellite_UpdateState(Satellite_t* obj, spektrum_data_t *merged_data);

uint8_t Satellite_UpdateControlMode(Satellite_t* obj, spektrum_data_t *merged_data);


#endif /* SATELLITE_RECEIVER_TASK_H_ */
