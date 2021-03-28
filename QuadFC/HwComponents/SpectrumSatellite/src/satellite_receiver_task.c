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

#include "FreeRTOS.h"
#include "task.h"
#include "Messages/inc/common_types.h"
#include "Components/Parameters/inc/paramHandler.h"

#include "SpectrumSatellite/inc/satellite_receiver_public.h"
#include "QuadFC/QuadFC_Serial.h"
#include "EventHandler/inc/event_handler.h"
#include "Messages/inc/common_types.h"
#include "Messages/inc/Msg_BindRc.h"
#include "Messages/inc/Msg_SpectrumData.h"
#include "Messages/inc/Msg_FlightMode.h"
#include "Messages/inc/Msg_CtrlMode.h"
#include "HAL/QuadFC/QuadFC_Gpio.h"
#include "Components/AppLog/inc/AppLog.h"

/*Include utilities*/
#include "Utilities/inc/my_math.h"


/**
 * @brief Handles a satellite receiver.
 *
 * The module that handles a satellite receiver. It will first build confidence in the
 * configuration since multiple configurations are supported. The transmitter-receiver
 * combination determines the frame configuration.
 *
 */

/// Defines used by the module.
#define SATELLITE_DATA_MASK 0x7ff            /*Data contained in the 11 least significant bits*/
#define SATELLITE_CHANNEL_MASK 0x7800        /*Channel number contained in the 4 bits above data.*/

#define BAUD_SATELLITE 115200

#define SATELLITE_MESSAGE_LENGTH 16

#define CHANNEL_CONFIDENCE (1)
#define FRAME_CONFIDENCE_MAX (15)
#define FRAME_CONFIDENCE_THRESHOLD (10)
#define MAX_NUMBER_OF_FRAME_TYPES (3)


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
 * Struct containing all the data the "object" needs.
 */
typedef struct Satellite
{
  spektrum_data_t* decoded_data;
  spectrum_config_t* configuration;
  uint8_t *satellite_receive_buffer;
  eventHandler_t* evHandler;
  paramHander_t* paramHandler;
  uint32_t bindMode;
  uint32_t uartNr;
  GpioName_t pwrCtrl;
  FlightMode_t          current_flight_mode_state;
  CtrlMode_t            current_control_mode;
}Satellite_t;

/**
 * Initialize the satellite module. Allocates all data fields needed by the task.
 * @param stateHandler
 * @param setpointHandler
 * @return Null if fail, pointer to a struct instance otherwise.
 */
Satellite_t* Satellite_Create(eventHandler_t* eventMaster, uint32_t uartNr, GpioName_t pwrCtrl, char index);

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


uint8_t Satellite_BindCB(eventHandler_t* obj, void* data, moduleMsg_t* eData);


uint8_t Satellite_FModeCB(eventHandler_t* obj, void* data, moduleMsg_t* eData);

uint8_t Satellite_CtrlModeCB(eventHandler_t* obj, void* data, moduleMsg_t* eData);

Satellite_t* Satellite_Create(eventHandler_t* eventHandler, uint32_t uartNr, GpioName_t pwrCtrl, char index)
{
    /* Create the queue used to pass things to the display task*/
    Satellite_t* taskParam = pvPortMalloc(sizeof(Satellite_t));
    taskParam->decoded_data = pvPortMalloc(sizeof(spektrum_data_t));
    taskParam->configuration = pvPortMalloc(sizeof(spectrum_config_t));
    taskParam->satellite_receive_buffer = pvPortMalloc(SATELLITE_MESSAGE_LENGTH * 2);
    taskParam->bindMode = 9;
    taskParam->uartNr = uartNr;
    taskParam->pwrCtrl = pwrCtrl;
    taskParam->evHandler = eventHandler;
    char name[] = {"Spectrum_X"};
    name[9]=index;
    taskParam->paramHandler = ParamHandler_CreateObj(1,eventHandler, name);
    taskParam->current_flight_mode_state = fmode_not_available;
    taskParam->current_control_mode = Control_mode_not_available;

    Event_RegisterCallback(taskParam->evHandler, Msg_BindRc_e, Satellite_BindCB, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_FlightMode_e, Satellite_FModeCB, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_CtrlMode_e, Satellite_CtrlModeCB, taskParam);

    Param_CreateObj(0, variable_type_uint32, readWrite, &taskParam->bindMode, "BindMode", ParamHandler_GetParam(taskParam->paramHandler));


    if( !taskParam || !taskParam->decoded_data || !taskParam->configuration
            || !taskParam->satellite_receive_buffer || !taskParam->evHandler)
    {
        return NULL;
    }
    //Scope so that the tmp structs live a short life.
    {
        spektrum_data_t tmpData = {{{0}}};
        *taskParam->decoded_data = tmpData;

        spectrum_config_t tmpConfig = {{{0}}};
        *taskParam->configuration = tmpConfig;
    }

    // Turn on the power to the module.
    Gpio_SetPinLow(taskParam->pwrCtrl);

    return taskParam;
}


/*void create_satellite_receiver_task(void)
 * Spectrum satellite receiver task. Initialize the USART instance
 * and create the task.
 */ 
void Satellite_CreateReceiverTask(  eventHandler_t* eventHandler, uint32_t uartNr, GpioName_t pwrCtrl, char index)
{
    uint8_t* receiver_buffer_driver = pvPortMalloc(
            sizeof(uint8_t) * SATELLITE_MESSAGE_LENGTH*2 );

    Satellite_t *SatelliteParam = Satellite_Create(eventHandler, uartNr, pwrCtrl, index);


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

    uint8_t rsp = QuadFC_SerialInit(SatelliteParam->uartNr, &opt);
    if(!rsp)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {

        }
    }
    char tmpName[] = "SpecSatX";
    tmpName[7] = index;

    /*Create the worker task*/
    xTaskCreate(  Satellite_ReceiverTask,   /* The task that implements the test. */
            tmpName,                        /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
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
    uint8_t *satellite_receive_buffer = param->satellite_receive_buffer;

    /*Like most tasks this is realized by a never-ending loop*/
    for ( ;; )
    {
        //Process incoming events.
        while(Event_Receive(param->evHandler, 0) == 1)
        {}

        /*Read data from the satellite receiver, wait longer than the period of the frames.*/

        QuadFC_Serial_t serialData = {0};
        serialData.buffer = satellite_receive_buffer;
        serialData.bufferLength = SATELLITE_MESSAGE_LENGTH;
        serialData.triggerLevel = SATELLITE_MESSAGE_LENGTH;
        bytes_read = QuadFC_SerialRead(&serialData, param->uartNr, satellite_block_time);

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
                    // We should not spam with setpoints when we are in init or config mode!
                    if(param->current_flight_mode_state != fmode_init && param->current_flight_mode_state != fmode_config)
                    {
                        moduleMsg_t* msg = Msg_SpectrumDataCreate(FC_Broadcast_e, 0, *decoded_data);

                        Event_Send(param->evHandler, msg);
                    }


                }
            }
        }

        // Sync by reading with a timeout smaller than that between two messages,
        // but longer than a single message.
        QuadFC_Serial_t serialDataSync;
        serialDataSync.buffer = satellite_receive_buffer;
        serialDataSync.bufferLength = SATELLITE_MESSAGE_LENGTH*2;
        uint32_t nr_bytes_received = QuadFC_SerialRead(&serialDataSync, param->uartNr, satellite_block_time_sync);

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

uint8_t Satellite_BindCB(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!obj || !data || !eData)
    {
        return 0;
    }
    uint8_t quitBind = Msg_BindRcGetQuit(eData);
    Satellite_t* satellite = (Satellite_t*)data; // data should always be the current handler.
    if(satellite->current_flight_mode_state == fmode_disarmed || satellite->current_flight_mode_state == fmode_fault)
    {
        if((satellite->bindMode == 9) || (satellite->bindMode == 10))
        {
            // Reconfigure uart rx to gpio.
            if(QuadFC_SerialReconfigPin(satellite->uartNr, pinConfigOutput))
            {
                QuadFC_SerialSetPin(satellite->uartNr, stateSet, rx);
                Gpio_SetPinHigh(satellite->pwrCtrl);
                vTaskDelay(pdMS_TO_TICKS(500));
                Gpio_SetPinLow(satellite->pwrCtrl);
                vTaskDelay(pdMS_TO_TICKS(100));

                if(quitBind != 1)
                {
                    LOG_ENTRY(FC_SerialIOtx_e,satellite->evHandler, "Binding RC.");
                    for (size_t i = 0; i < satellite->bindMode; i++)
                    {
                        QuadFC_SerialSetPin(satellite->uartNr, stateReset, rx);
                        vTaskDelay(pdMS_TO_TICKS(2));

                        QuadFC_SerialSetPin(satellite->uartNr, stateSet, rx);
                        vTaskDelay(pdMS_TO_TICKS(2));
                    }
                }
                else
                {
                    LOG_ENTRY(FC_SerialIOtx_e,satellite->evHandler, "Quitting binding process.");
                }
                
                // Reconfigure gpio back to rx.
                QuadFC_SerialReconfigPin(satellite->uartNr, pinConfigStandard);
            }
            else
            {
                LOG_ENTRY(FC_SerialIOtx_e,satellite->evHandler, "Not able to re-configure pin.");
                //TODO return not supported message.
            }
            
        }
        else
        {
            LOG_ENTRY(FC_SerialIOtx_e,satellite->evHandler, "Unsupported bind mode. Please configure a different bind mode.");
            //TODO return not supported message.
        }
        

    }
    else
    {
        LOG_ENTRY(FC_SerialIOtx_e,satellite->evHandler, "Only allowed to enter bind in disarmed mode.");
    }
    return 1;
}

uint8_t Satellite_FModeCB(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!obj || !data || !eData)
    {
        return 0;
    }
    Satellite_t* satellite = (Satellite_t*)data; // data should always be the current handler.
    satellite->current_flight_mode_state =  Msg_FlightModeGetMode(eData);
    return 1;
}

uint8_t Satellite_CtrlModeCB(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!obj || !data || !eData)
    {
        return 0;
    }
    Satellite_t* satellite = (Satellite_t*)data; // data should always be the current handler.
    satellite->current_control_mode = Msg_CtrlModeGetMode(eData);
    return 1;
}