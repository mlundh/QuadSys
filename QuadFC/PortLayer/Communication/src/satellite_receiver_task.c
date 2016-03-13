/*----------------------------------------------------------------------
 * receiver_task.c
 *
 * QuadFC - Firmware for a quadcopter.
 * Copyright (C) 2014  Martin Lundh
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
 * */

/* ----------------------------------------------------------------------
 * Spectrum satellite receiver task.
 * ----------------------------------------------------------------------
 * Code to parse the serial data received from the spectrum 
 * satellite RC receiver in conjunction with spectrum DX6i transmitter.
 * 
 * Each channel uses two bytes. The data is contained in the
 * 11 least significant bits and channel number is contained
 * in the 4 bits over that.
 *
 * That is one channel looks as: [0 C C C C D D D D D D D D D D D]
 * Where "D" is data and "C" is channel.
 * 
 *
 */ 
#include "PortLayer/Communication/inc/satellite_receiver_task.h"
#include "PortLayer/Communication/inc/satellite_receiver_public.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "HMI/inc/led_control_task.h"
#include "InternalStateHandler/inc/state_handler.h"
#include "SetpointHandler/inc/setpoint_handler.h"

/*Include utilities*/
#include "Utilities/inc/my_math.h"

Satellite_t* Satellite_Init(StateHandler_t* stateHandler, SpObj_t* setpointHandler)
{
  /* Create the queue used to pass things to the display task*/
  Satellite_t* taskParam = pvPortMalloc(sizeof(Satellite_t));
  taskParam->decoded_data = pvPortMalloc(sizeof(spektrum_data_t));
  taskParam->configuration = pvPortMalloc(sizeof(spectrum_config_t));
  taskParam->setpoint = pvPortMalloc(sizeof(state_data_t));
  taskParam->satellite_receive_buffer = pvPortMalloc(SATELLITE_MESSAGE_LENGTH * 2);
  taskParam->setpointHandler = setpointHandler;
  taskParam->stateHandler = stateHandler;
  taskParam->divisor = 1;
  taskParam->multiplier = 1;
  taskParam->xMutexParam = xSemaphoreCreateMutex();

  param_obj_t * ReceiverRoot = Param_CreateObj(2, NoType, readOnly, NULL, "Rcver", Param_GetRoot(), NULL);

   // Enable receiver sensitivity adjustment.
  Param_CreateObj(0, int32_variable_type, readWrite, &taskParam->multiplier, "mult", ReceiverRoot, taskParam->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite, &taskParam->divisor, "div", ReceiverRoot, taskParam->xMutexParam);

  if( !taskParam || !taskParam->decoded_data || !taskParam->configuration || !taskParam->setpoint
      || !taskParam->satellite_receive_buffer || !taskParam->xMutexParam)
  {
    return NULL;
  }
  //Scope so that the tmp structs live a short life.
  {
    spektrum_data_t tmpData = {{{0}}};
    *taskParam->decoded_data = tmpData;

    spectrum_config_t tmpConfig = {{{0}}};
    *taskParam->configuration = tmpConfig;

    state_data_t tmpSetpoint = {{0}};
    *taskParam->setpoint = tmpSetpoint;
  }

  return taskParam;
}


/*void create_satellite_receiver_task(void)
 * Spectrum satellite receiver task. Initialize the USART instance
 * and create the task.
 */ 
void Satellite_CreateReceiverTask(  StateHandler_t* stateHandler, SpObj_t* setPointHandler )
{
  uint8_t* receiver_buffer_driver = pvPortMalloc(
      sizeof(uint8_t) * SATELLITE_MESSAGE_LENGTH*2 );

  Satellite_t *SatelliteParam = Satellite_Init(stateHandler, setPointHandler);


  if(!SatelliteParam || !receiver_buffer_driver)
  {
    for ( ;; );
  }

  QuadFC_SerialOptions_t opt = {
      BAUD_SATELLITE,
      EightDataBits,
      NoParity,
      OneStopBit,
      NoFlowControl,
      receiver_buffer_driver,
      SATELLITE_MESSAGE_LENGTH*2
  };

  uint8_t rsp = QuadFC_SerialInit(SATELITE_USART, &opt);
  if(!rsp)
  {
    /*Error - Could not create serial interface.*/

    for ( ;; )
    {

    }
  }

  /*Create the worker task*/
  xTaskCreate(	Satellite_ReceiverTask,   /* The task that implements the test. */
      "Satellite",                        /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
      500,                                /* The size of the stack allocated to the task. */
      (void *) SatelliteParam,            /* The parameter is used to pass the already configured USART port into the task. */
      configMAX_PRIORITIES-2,             /* The priority allocated to the task. */
      NULL);                              /* Used to store the handle to the created task - in this case the handle is not required. */
}


/*

 */
void Satellite_ReceiverTask(void *pvParameters)
{
  /**
   * One frame each 11 or 22 ms. This causes an error message
   * if no valid rc-signal is detected. 25ms between reads.
   */
  const portBASE_TYPE satellite_block_time = 25UL / portTICK_PERIOD_MS;
  /**
   *  One frame each 11 ms.
   */
  const portBASE_TYPE satellite_block_time_sync = 5UL / portTICK_PERIOD_MS;


  uint8_t bytes_read = 0;

  Satellite_t* param = (Satellite_t*)pvParameters;
  spektrum_data_t* decoded_data = param->decoded_data;
  spectrum_config_t* configuration = param->configuration;
  state_data_t* setpoint = param->setpoint;
  uint8_t *satellite_receive_buffer = param->satellite_receive_buffer;
  SpObj_t* setpointHandler = param->setpointHandler;

  /*Like most tasks this is realized by a never-ending loop*/
  for ( ;; )
  {
    /*Read data from the satellite receiver, wait longer than the period of the frames.*/

    QuadFC_Serial_t serialData;
    serialData.buffer = satellite_receive_buffer;
    serialData.bufferLength = SATELLITE_MESSAGE_LENGTH;
    bytes_read = QuadFC_SerialRead(&serialData, SATELITE_USART, satellite_block_time);

    /*If 16 bytes are read then all is ok, otherwise skip. */
    if ((bytes_read == SATELLITE_MESSAGE_LENGTH))
    {
      int ret = Satellite_DecodeSignal(satellite_receive_buffer, configuration, decoded_data);
      if (ret)
      {
        decoded_data->channels_merged |= decoded_data->channels_last_read;

        // If all channels in the configuration are present then we have a complete setpoint.
        if (decoded_data->channels_merged == configuration->channels_available)
        {
          // We have a complete set of channels.
          Satellite_MapToSetpointRate(param, decoded_data, setpoint);
          SpHandl_SetSetpoint(setpointHandler, setpoint, MESSAGE_VALID_FOR_MS, 1);
          Satellite_UpdateState(param, decoded_data);

        }
      }
    }

    // Sync by reading with a timeout smaller than that between two messages,
    // but longer than a single message.
    QuadFC_Serial_t serialDataSync;
    serialDataSync.buffer = satellite_receive_buffer;
    serialDataSync.bufferLength = SATELLITE_MESSAGE_LENGTH*2;
    uint32_t nr_bytes_received = QuadFC_SerialRead(&serialDataSync, SATELITE_USART, satellite_block_time_sync);

    if (nr_bytes_received >=SATELLITE_MESSAGE_LENGTH)
    {
      /*no timeout occurred, error!*/
    }
  }
}

int Satellite_DecodeSignal(uint8_t* data, spectrum_config_t* configuration, spektrum_data_t* spectrum_data)
{
  uint32_t raw_data;
  uint32_t channel;
  uint32_t channel_data;
  // Check uses the individual bits to determine if the channel has been seen.
  uint16_t channels_in_current_frame = 0;

  // Create a working copy where we can store the extracted channel data temporarily.
  // We are using a local copy to allow multiple frames to be merged, at the same time
  // as we make sure that we can safely discard any faulty frames.
  spektrum_channel_t l_channel_data[NUMBER_OF_CHANNELS] = {{0}};

  /*
   * Each channel uses two bytes of information as described in the above comment.
   * The first two bytes (0 & 1) are headers with quality factors etc.
   */
  for (int i=2; i<16; i += 2)
  {
    // Extract data and channel info.
    raw_data = ((uint32_t)data[i]<<8) | ((uint32_t)data[i+1]);    // Combine the two bytes of data that represents one channel.
    channel = (raw_data & SATELLITE_CHANNEL_MASK) >> 11;          // Get the channel number.
    channel_data = (raw_data & SATELLITE_DATA_MASK);              // Get the data.

    // Verify that the data is valid and that we have not seen this before.

    /*
     * Channel 15 represent unused fields in the frame. All channel numbers
     * should be less then that.
     */
    if (channel < NUMBER_OF_CHANNELS)
    {
      if (! (channels_in_current_frame & (1 << channel)))
      {
        // The current channel has not been set while updating this frame.
        // (All is fine if the bit in channels_in_current_frame is set to 0.)
        l_channel_data[channel].value = channel_data;
        channels_in_current_frame |= 1 << channel;
      }
      else
      {
        // If something went wrong and we have already seen this channel.
        return 0;
      }
    }
  }

  if (configuration->confident_frame_config)
  {
    // We have a known set of frames. Anything other then that should be discarded.
    uint8_t known = 0;
    for (int i = 0; i < MAX_NUMBER_OF_FRAME_TYPES; i++)
    {
      if ((channels_in_current_frame == configuration->frame_types_present[i].channels) && !known)
      {
        // This channel configuration is known.
        known = 1;
      }
    }
    if (!known)
    {
      // The frame configuration is unknown and should not be used.
      return 0;
    }
    // At this point we know that the frame matches what we expect and that the data can be trusted.
    // Copy the extracted data to the real data structure.
    for (int channel_nr = 0; channel_nr < NUMBER_OF_CHANNELS; channel_nr++)
    {
      if( 0 < (channels_in_current_frame & (1 << channel_nr)))
      {
        spectrum_data->ch[channel_nr].value = l_channel_data[channel_nr].value;
      }
    }
    spectrum_data->channels_last_read = channels_in_current_frame;
  }
  else
  {
    // No fixed frame config, try to lock a frame config.
    Satellite_LockFrameType(configuration, channels_in_current_frame);
    // We should not use the frame since we do not have a confident configuration.
    return 0;
  }
  return 1;
}

uint8_t Satellite_LockFrameType(spectrum_config_t* configuration, uint16_t channels_in_current_frame)
{
  // Validate the input. This function should not be called if we have a valid configuration.
  if (configuration->confident_frame_config)
  {
    return 0;
  }
  /*
   * Build confidence in the frame setup.
   * The process:
   * 1. Check if the frame configuration has been seen before. Increase confidence if it has.
   * 2. If it is a new or unknown configuration, try adding it to an unused slot. (Where count is 0.)
   * 3. Restart validation of all frames below a threshold if all slots are occupied.
   */

  // We have not yet reached a state where we can trust the frames.
  uint8_t match = 0;
  for (int i = 0; i < MAX_NUMBER_OF_FRAME_TYPES; i++)
  {
    // Loop through all the stored frame configurations.
    if (channels_in_current_frame == configuration->frame_types_present[i].channels)
    {
      // A frame with the same channel setup has been received before. Good!
      match = 1;
      // Increase confidence in the frame configuration if it hasn't maxed out already.
      if (configuration->frame_types_present[i].confidence < FRAME_CONFIDENCE_MAX)
      {
        configuration->frame_types_present[i].confidence++;
        if(configuration->frame_types_present[i].confidence >= FRAME_CONFIDENCE_MAX)
        {
          // One frame has reached max confidence. Generate a list of all available channels.
          configuration->confident_frame_config = 1;
          for (int j = 0; j < MAX_NUMBER_OF_FRAME_TYPES; j++)
          {
            // Loop through all the stored frame configurations and store all availible channels.
            if (configuration->frame_types_present[j].confidence > FRAME_CONFIDENCE_THRESHOLD)
            {
              configuration->channels_available |= configuration->frame_types_present[j].channels;
            }
          }
        }
      }
    }
  }

  uint8_t ret = 0;
  if (match)
  {
    ret = 1; // We already found the frame. Good!
  }
  else
  {
    // This frame configuration is unknown, try to add it.
    for (int i = 0; i < MAX_NUMBER_OF_FRAME_TYPES; i++)
    {
      // Loop through all the stored frame configurations.
      if (configuration->frame_types_present[i].confidence == 0 && !ret)
      {
        // An available slot is found.
        configuration->frame_types_present[i].channels = channels_in_current_frame;
        configuration->frame_types_present[i].confidence = 1;
        // This is a valid state, indicate this.
        ret = 1;
      }
    }
  }

  if(!ret)
  {
    // Too many frames of different configurations seen. Restarting validation...
    for (int i = 0; i < MAX_NUMBER_OF_FRAME_TYPES; i++)
    {
      // Loop through all the stored frame configurations.
      // Reset frames if the threshold is not met.
      if (configuration->frame_types_present[i].confidence < FRAME_CONFIDENCE_THRESHOLD)
      {
        configuration->frame_types_present[i].channels = 0;
        configuration->frame_types_present[i].confidence = 0;
      }
    }
  }
  return ret;
}

void Satellite_MapToSetpointRate(Satellite_t* obj, spektrum_data_t *reciever_data, state_data_t *setpoint)
{
  if( xSemaphoreTake( obj->xMutexParam, ( TickType_t )(1UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    setpoint->state_vector[z_vel]               = reciever_data->ch[0].value; // THRO
    setpoint->state_vector[yaw_rate]            = ((reciever_data->ch[3].value - SATELLITE_CH_CENTER) * obj->multiplier); // YAW
    setpoint->state_vector[pitch_rate]          = ((reciever_data->ch[2].value - SATELLITE_CH_CENTER) * obj->multiplier); // PITCH
    setpoint->state_vector[roll_rate]           = ((reciever_data->ch[1].value - SATELLITE_CH_CENTER) * obj->multiplier); // ROLL

    if(obj->divisor > 0)
    {
      setpoint->state_vector[yaw_rate]  = (setpoint->state_vector[yaw_rate]  / obj->divisor);
      setpoint->state_vector[pitch_rate]= (setpoint->state_vector[pitch_rate]/ obj->divisor);
      setpoint->state_vector[roll_rate] = (setpoint->state_vector[roll_rate] / obj->divisor);
    }
    xSemaphoreGive(obj->xMutexParam);
  }
  return;
}

uint8_t Satellite_UpdateState(Satellite_t* obj, spektrum_data_t *merged_data)
{
  /*Disarm requested*/
  uint8_t ret = 1;
  if ((merged_data->ch[4].value > SATELLITE_CH_CENTER) // Flap is on
      && (merged_data->ch[0].value < 40)) // throttle is low
  {
    if(pdTRUE == State_Change(obj->stateHandler, state_disarming))
    {
    }
  }
  /*Arming request*/
  else if ((merged_data->ch[4].value < SATELLITE_CH_CENTER) // Flap is off
      && (merged_data->ch[0].value < 40)) // throttle is low
  {
    if(pdTRUE == State_Change(obj->stateHandler, state_arming))
    {
    }
  }
  return ret;
}
