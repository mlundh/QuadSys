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
 * That is one frame looks as: [0 C C C C D D D D D D D D D D D]
 * Where "D" is data and "C" is channel.
 * 
 *
 */ 
#include "satellite_receiver_task.h"

#include "led_control_task.h"

int satellite_decode_signal(uint8_t data[], spektrum_data_t *decoded_data);
static uint8_t receive_buffer[SATELLITE_MESSAGE_LENGTH*2];

/*void create_satellite_receiver_task(void)
 * Spectrum satellite receiver task. Initialize the USART instance
 * and create the task.
 *
 */ 
void create_satellite_receiver_task(void)
{
    
    freertos_usart_if freertos_usart;
    
    freertos_peripheral_options_t driver_options = {
        receive_buffer,									/* The buffer used internally by the USART driver to store incoming characters. */
        SATELLITE_MESSAGE_LENGTH,     					/* The size of the buffer provided to the USART driver to store incoming characters. */
        (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3),	/* The priority used by the USART interrupts. */
        USART_RS232,									/* Configure the USART for RS232 operation. */
        (WAIT_TX_COMPLETE|WAIT_RX_COMPLETE)				/* Wait for Tx and Rx to complete before returning from functions. */
    };

    const sam_usart_opt_t usart_settings = {
        BAUD_SATELLITE,                                 /* Speed of the transfer, baud*/
        US_MR_CHRL_8_BIT,                               /* 8 bit transfer*/
        US_MR_PAR_NO,                                   /* No parity*/
        US_MR_NBSTOP_1_BIT,                             /* One stop bit*/
        US_MR_CHMODE_NORMAL,                            /* */
        0                                               /* Only used in IrDA mode. */
    };

    /* Initialize the USART interface. */
    freertos_usart = freertos_usart_serial_init(SATELITE_USART,
                                                &usart_settings,
                                                &driver_options);
    if (freertos_usart == NULL)
    {
        for (;;)
        {

        }
    }
    
    /*Create the worker task*/
	xTaskCreate(	satellite_receiver_task,					    /* The task that implements the test. */
					NULL,	                            /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
					500,							    /* The size of the stack allocated to the task. */
					(void *) freertos_usart,		    /* The parameter is used to pass the already configured USART port into the task. */
					configMAX_PRIORITIES-2,			    /* The priority allocated to the task. */
		    		NULL);							    /* Used to store the handle to the created task - in this case the handle is not required. */
}


/* void satellite_receiver_task(void *pvParameters)
 *
 * Task communicating with the satellite receiver. 
 *
 * it is necessary to sync communication with the satellite receiver to
 * ensure that each frame is captured as a whole. A frame consists of 
 * 16 consecutive bytes that should be received without delay. A new 
 * frame should arrive every 11 or 22 ms.

 */
void satellite_receiver_task(void *pvParameters)
{
   
    
  uint8_t satellite_receive_buffer[SATELLITE_MESSAGE_LENGTH];
  const portBASE_TYPE satellite_block_time = 75UL / portTICK_RATE_ONE_THIRD_MS;	        // One frame each 11 or 22 ms. This causes an error message if no valid rc-signal is detected. 75/3 = 25ms between reads
  const portBASE_TYPE satellite_block_time_sync = 15UL / portTICK_RATE_ONE_THIRD_MS;		// One frame each 11 ms


  uint8_t bytes_read = 0;
  uint8_t timeout_ok = 0;
  spektrum_data_t decoded_data;
  spektrum_channels_t merged_data;
  receiver_data_t satellite_data_legacy;
    
  /*The already open usart port is passed through the task parameters*/
  freertos_usart_if spectrum_usart = (freertos_usart_if)pvParameters;

  /* Initialize the data structure that holds the decoded reciever data */
  decoded_data.ch_count = sizeof(decoded_data.ch)/sizeof(decoded_data.ch[0]);
  decoded_data.frame_type_count = sizeof(decoded_data.frame_type)/sizeof(decoded_data.frame_type[0]);
  for (int i = 0; i < decoded_data.ch_count; i++) {
    decoded_data.ch[i].value = 0;
  }
  for (int i = 0; i < decoded_data.frame_type_count; i++) {
    decoded_data.frame_type[i].channels = 0;
    decoded_data.frame_type[i].confidence = 0;
  }
  decoded_data.confident_frame_types = false;
  decoded_data.channels_available = 0;

  /* Initialize the data structure where we merg data from multiple reciever frames */
  merged_data.ch_count          = sizeof(merged_data.ch)/sizeof(merged_data.ch[0]);
  merged_data.channels_merged   = 0;
  merged_data.updated = false;
  merged_data.frames_needed = 0;
  for (int i = 0; i < merged_data.ch_count; i++) {
    merged_data.ch[i].value = 0;
  }

  /*Like most tasks this is realized by a never-ending loop*/
    
  // TODO add check that the queue is empty
  for ( ;; )
  {
    // LED
    //uint8_t led_state = clear_error_led2;
    //xQueueSendToBack(xQueue_led, &led_state, mainDONT_BLOCK);
    
    if (true == merged_data.updated)
    {
      merged_data.channels_merged = 0;
      merged_data.updated         = false;
      merged_data.frames_needed   = 0;
      for (int i = 0; i < merged_data.ch_count; i++) {
        merged_data.ch[i].value = 0;
      }
    }
      
    /*Read data from the satellite receiver, wait longer than the period of the frames.*/
    bytes_read = freertos_usart_serial_read_packet(spectrum_usart, satellite_receive_buffer, SATELLITE_MESSAGE_LENGTH, satellite_block_time);
    merged_data.frames_needed++;
    
    /*If not 16 bytes are read something is wrong */
    if ((bytes_read != SATELLITE_MESSAGE_LENGTH))
    {
      /*error!*/
      decoded_data.channels_last_read = 0;
      // Send incomplete data to the FC indicating that there is an error.
      satellite_data_legacy = dx7s_to_legacy_fc_channels(&merged_data);
      xQueueSendToBack(xQueue_receiver, &satellite_data_legacy, mainDONT_BLOCK);
    }
    else
    {
      int ret = satellite_decode_signal(satellite_receive_buffer, &decoded_data);
      if (0 == ret)
      {
        for (int i = 0; i < merged_data.ch_count; i++) {
          if (0 < (decoded_data.channels_last_read & (1 << i)))
          {
            merged_data.ch[i].value        = decoded_data.ch[i].value;
          }
        }
        merged_data.channels_merged |= decoded_data.channels_last_read;
        if (merged_data.channels_merged == decoded_data.channels_available)
        {
          // We have a complete set of channels.
          merged_data.updated = true;
          
          // LED
          //uint8_t led_state = warning_lost_com_message;
          //xQueueSendToBack(xQueue_led, &led_state, mainDONT_BLOCK);
          
          // Convert to the old format used by the flight controller.
          satellite_data_legacy = dx7s_to_legacy_fc_channels(&merged_data);
          // Send the data to the back of the queue
          xQueueSendToBack(xQueue_receiver, &satellite_data_legacy, mainDONT_BLOCK);
        } else if (merged_data.frames_needed > 2) {
          // Send incomplete data to the FC indicating that there is an error.
          satellite_data_legacy = dx7s_to_legacy_fc_channels(&merged_data);
          xQueueSendToBack(xQueue_receiver, &satellite_data_legacy, mainDONT_BLOCK);
        }
      }
      else
      {
        // Send incomplete data to the FC indicating that there is an error.
        satellite_data_legacy = dx7s_to_legacy_fc_channels(&merged_data);
        xQueueSendToBack(xQueue_receiver, &satellite_data_legacy, mainDONT_BLOCK);
        // Something went wrong parsing the data
        if (ret == -1)
        {
          // Check return values...
        }
      }
    }

    timeout_ok = freertos_usart_serial_read_packet(spectrum_usart, satellite_receive_buffer,
    SATELLITE_MESSAGE_LENGTH*2,
    satellite_block_time_sync);
    if (timeout_ok >=SATELLITE_MESSAGE_LENGTH)
    {
      /*no timeout occurred, error!*/
    }
  }
}


/* -------------------------------------------------------------------
 * Helper functions
 *--------------------------------------------------------------------
 */

/* void satellite_decode_signal(uint8_t data[], receiver_data_t *decoded_data)
 * 
 * Data is written into the struct struct receiver_data *decoded_data, if
 * the field decoded_data->connection_ok is 0 then the decoding was sucsessful,
 * otherwise something is wrong and values in the struct should be discarded. 
 * 
 */
int satellite_decode_signal(uint8_t data[], spektrum_data_t *decoded_data){
  uint32_t raw_data;
  uint32_t channel;
  uint32_t channel_data;
  uint16_t check = 0;
  int ret = 0;
    
  // Create a working copy where we can store the extracted channel data temporarily.
  spektrum_channel_t l_channel_data[decoded_data->ch_count];
  for (int i = 0; i < decoded_data->ch_count; i++) {
    l_channel_data[i].value = 0;
  }
    
  /******************************************************************
  * Check that the frame we recieved match one of the expected ones.
  ******************************************************************/

  /*Each channel uses two bytes of information as described in the above comment.*/
  for (int i=2; i<16; i += 2) // The first two bytes (0 & 1 are headers with quality factors etc.)
  {
    raw_data = ((uint32_t)data[i]<<8) | ((uint32_t)data[i+1]);		// Combine the two bytes of data that represents one channel.
    channel = (raw_data & SATELLITE_CHANNEL_MASK) >> 11;			// Get the channel.
    channel_data = (raw_data & SATELLITE_DATA_MASK);				// Get the data.
      
    if (channel < 15) // Channel 15 represent unused fields in the frame. All channel numbers should be less then that.
    {
      if (!(check & (1 << channel))) // Channel has not been set while updating this frame. (All is fine if the bit is set to 0.)
      {
        l_channel_data[channel].value = channel_data;
        check |= 1 << channel;
      }
      else
      {
        // If something went wrong and we see a chennel more then once.
        ret = -1;
      }
    }
  }
    
  if (decoded_data->confident_frame_types) {
    // We have a known set of frames. Anything other then that should be discarded.
    bool known = false;
    for (int i = 0; i < decoded_data->frame_type_count; i++)
    {
      if ((check == decoded_data->frame_type[i].channels) && (decoded_data->frame_type[i].confidence > FRAME_CONFIDENCE_THRESHOLD)) {
        // This channel configuration is known.
        known = true;
      }
    }
    if (!known) {
      // The frame configuration is unknown and should not be used.
      ret = -2;
      return ret;
    }
  }
    
  // At this point we know that the frame matches what we expect and that the data can be trusted.
  // Copy the extracted data to the real data structure and update the confidence.
  for (int channel_nr = 0; channel_nr < decoded_data->ch_count; channel_nr++) {
    if( 0 < (check & (1 << channel_nr)))
    {
      decoded_data->ch[channel_nr].value = l_channel_data[channel_nr].value;
    }
  }
  decoded_data->channels_last_read = check;


  /*
   * Build confidence in the frame setup.
   * The process:
   * 1. Check if the frame configuration has been seen before. Increase confidence if it has.
   * 2. If it is a new or unknown configuration, try adding it to an unused slot. (Where count is 0.)
   * 3. Retart validation of all frames below a threshold if all slots are occupied.    
  */
  if (!decoded_data->confident_frame_types)
  {
    // We have not yet reached a state where we can trust the frames.
    bool match = false;
    for (int i = 0; i < decoded_data->frame_type_count; i++)
    {
      // Loop through all the stored frame configurations.
      if (check == decoded_data->frame_type[i].channels) {
        // A frame with the same channel setup has been recived before. Good!
        if (decoded_data->frame_type[i].confidence < FRAME_CONFIDENCE_MAX)
        {
          // Increase confidence in the frame configuration if it hasn't maxed out already.
          decoded_data->frame_type[i].confidence++;
          if (decoded_data->frame_type[i].confidence == FRAME_CONFIDENCE_MAX)
          {
            // One frame has reached max confidence. Generate a list of all available channels.
            decoded_data->confident_frame_types = true;
            for (int j = 0; j < decoded_data->frame_type_count; j++)
            {
              // Loop through all the stored frame configurations.
              if (decoded_data->confident_frame_types && (decoded_data->frame_type[j].confidence > FRAME_CONFIDENCE_THRESHOLD))
              {
                decoded_data->channels_available |= decoded_data->frame_type[j].channels;
              }
            }
          }
        }
        match = true;
      }
    }
    if (match) {
      return ret; // We already found the frame. Good!
    }
    
    // This frame configuration is unknown, try to add it.
    for (int i = 0; i < decoded_data->frame_type_count; i++)
    {
      // Loop through all the stored frame configurations.
      if (decoded_data->frame_type[i].confidence == 0) {
        // An available slot is found. Add then return.
        decoded_data->frame_type[i].channels = check;
        decoded_data->frame_type[i].confidence = 1;
        return ret;
      }
    }
    
    // Too many frames of different configurations seen. Restarting validation...
    for (int i = 0; i < decoded_data->frame_type_count; i++)
    {
      // Loop through all the stored frame configurations.
      if (decoded_data->frame_type[i].confidence < FRAME_CONFIDENCE_THRESHOLD) // Reset frames if the threshold is not met.
      {
        decoded_data->frame_type[i].channels = 0;
        decoded_data->frame_type[i].confidence = 0;
      }
    }
  }
  return ret;
}

/**********************************************************************
 * Function to map into a new format the FC will understand eventually.
 **********************************************************************/

fc_channels_t dx7s_to_fc_channels(spektrum_channels_t *reciever_channels) {
  fc_channels_t control_channels;
  control_channels.stick_left_up_down.value         = 32*reciever_channels->ch[0].value;
  control_channels.stick_left_up_down.updated       = (0 < (reciever_channels->channels_merged & (1 << 0))) ? reciever_channels->updated : false;
  
  control_channels.stick_left_right_left.value      = 32*reciever_channels->ch[3].value;
  control_channels.stick_left_right_left.updated    = (0 < (reciever_channels->channels_merged & (1 << 3))) ? reciever_channels->updated : false;
  
  control_channels.stick_right_up_down.value        = 32*reciever_channels->ch[2].value;
  control_channels.stick_right_up_down.updated      = (0 < (reciever_channels->channels_merged & (1 << 2))) ? reciever_channels->updated : false;
  
  control_channels.stick_right_right_left.value     = 32*reciever_channels->ch[1].value;
  control_channels.stick_right_right_left.updated   = (0 < (reciever_channels->channels_merged & (1 << 1))) ? reciever_channels->updated : false;
  
  control_channels.aux[0].value                     = 32*reciever_channels->ch[5].value; // FLAP
  control_channels.aux[0].updated                   = (0 < (reciever_channels->channels_merged & (1 << 5))) ? reciever_channels->updated : false;
  
  control_channels.aux[1].value                     = 32*reciever_channels->ch[4].value; // GEAR
  control_channels.aux[1].updated                   = (0 < (reciever_channels->channels_merged & (1 << 4))) ? reciever_channels->updated : false;
  
  control_channels.aux[2].value                     = 32*reciever_channels->ch[6].value; // AUX2
  control_channels.aux[2].updated                   = (0 < (reciever_channels->channels_merged & (1 << 6))) ? reciever_channels->updated : false;
  
  control_channels.aux_channel_count = sizeof(control_channels.aux)/sizeof(control_channels.aux[0]);
  
  for (int i = 3; i > control_channels.aux_channel_count; i++)
  {
    control_channels.aux[i].updated = false;
  }
  
  return control_channels;
}

/*****************************************************
 * Function to map into a legacy format the FC
 * will understand. (adapt to the current FC format.)
 *****************************************************/

receiver_data_t dx7s_to_legacy_fc_channels(spektrum_channels_t *reciever_channels) {
  receiver_data_t control_channels;
  control_channels.ch0            = reciever_channels->ch[0].value; // THRO
  control_channels.ch3            = reciever_channels->ch[3].value; // YAW
  control_channels.ch2            = reciever_channels->ch[2].value; // PITCH
  control_channels.ch1            = reciever_channels->ch[1].value; // ROLL
  control_channels.ch5            = reciever_channels->ch[4].value; // FLAP
  control_channels.ch6            = reciever_channels->ch[5].value; // GEAR
  control_channels.connection_ok  = reciever_channels->updated;
  
  //reciever_channels->updated = false;
  return control_channels;
}
