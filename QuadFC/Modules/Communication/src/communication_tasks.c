/*
 * communication_tasks.c
 *
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
 */

//#define DEBUG // uncomment this line to enable debug prints.

#include "string.h"

#include "Communication/inc/communication_tasks.h"
#include "Parameters/inc/paramHandler.h"
#include "Debug/inc/debug_handler.h"
#include "Messages/inc/Msg_Parser.h"
#include "Messages/inc/Msg_Transmission.h"
#include "BoardConfig.h"
#include "Messages/inc/Msg_Log.h"



#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "Components/SLIP/inc/slip_packet.h"
#include "Components/SLIP/inc/crc.h"
#include "Components/AppLog/inc/AppLogHandler.h"
#include "Components/AppLog/inc/AppLog.h"
#include "Parameters/inc/parameters.h"
#include "QuadFC/QuadFC_Memory.h"
#include "QuadFC/QuadFC_Serial.h"
#include "QuadFC/QuadFC_SPI.h"
#include "Log/inc/logHandler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/* Use USART1 (labeled RX2 17 and TX2 16)*/
#define COM_RECEIVE_BUFFER_LENGTH       (512)

#define COM_PACKET_LENGTH_MAX       512
#define NR_RETRANSMITT (3)


/*Global crc table*/
uint16_t *crcTable;


typedef struct RxCom
{
    QuadFC_Serial_t * serialBuffer;
    uint8_t messageBuffer[COM_PACKET_LENGTH_MAX];
    SLIP_t *SLIP;                   //!< RxTask owns SLIP package.
    SLIP_Parser_t* parser;
    uint8_t receive_buffer[COM_RECEIVE_BUFFER_LENGTH];
    param_helper_t *helper;
    param_helper_t *saveHelper;
    FlightModeHandler_t* stateHandler;
    eventHandler_t* evHandler;
    LogHandler_t* logHandler;
    paramHander_t* paramHandler;
    debug_handler_t* debugHandler;

}RxCom_t;


typedef struct TxCom
{
    SLIP_t *txSLIP;              //!< Handle for SLIP packet, frame that will be transmitted.
    eventHandler_t* evHandler;
    uint8_t msgNr;
    TransmissionCtrl transmission;
    uint8_t serializeBuffer[COM_PACKET_LENGTH_MAX];
    uint32_t serializebufferSize;
    AppLogHandler_t* AppLogHandler;

}TxCom_t;


/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
TxCom_t* Com_InitTx(eventHandler_t* eventHandler);

/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
RxCom_t* Com_InitRx(eventHandler_t* eventHandler);


/**
 * FreeRTOS task handling transmission of messages.
 * @param pvParameters
 */
static void Com_TxTask( void *pvParameters );

uint8_t Com_TxSend(eventHandler_t* obj, void* data, moduleMsg_t* msg);

/**
 * FreeRTOS task handling the reception of messages.
 * @param pvParameters
 */
static void Com_RxTask( void *pvParameters );


uint8_t Com_TxTransmission(eventHandler_t* obj, void* data, moduleMsg_t* msg);

uint8_t Com_HandleLog(eventHandler_t* obj, void* data, moduleMsg_t* msg);

TxCom_t* Com_InitTx(eventHandler_t* eventHandler)
{
    TxCom_t* taskParam = pvPortMalloc(sizeof(TxCom_t));
    if(!taskParam)
    {
        return NULL;
    }
    taskParam->txSLIP =  Slip_Create(COM_PACKET_LENGTH_MAX);
    taskParam->evHandler = eventHandler;
    taskParam->msgNr = 0;
    taskParam->transmission = transmission_OK;
    taskParam->serializeBuffer[0] = '\0';
    taskParam->serializebufferSize = COM_PACKET_LENGTH_MAX;
    taskParam->AppLogHandler = AppLogHandler_Create(eventHandler);

    if( !taskParam->txSLIP || !taskParam->evHandler)
    {
        return NULL;
    }

    // Subscribe to the events that the task is interested in. All events that can be sent over the uart link.

    Event_RegisterPortFunction(taskParam->evHandler, Com_TxSend, taskParam);

    Event_RegisterCallback(taskParam->evHandler, Msg_Transmission_e, Com_TxTransmission, taskParam);

    return taskParam;
}


RxCom_t* Com_InitRx(eventHandler_t* eventHandler)
{
    /* Create the queue used to pass things to the display task*/
    RxCom_t* taskParam = pvPortMalloc(sizeof(RxCom_t));
    if(!taskParam)
    {
        return NULL;
    }
    taskParam->debugHandler = Debug_CreateHandler(eventHandler);
    taskParam->helper = pvPortMalloc(sizeof(param_helper_t));
    taskParam->saveHelper = pvPortMalloc(sizeof(param_helper_t));
    taskParam->SLIP =  Slip_Create(COM_PACKET_LENGTH_MAX);
    taskParam->evHandler = eventHandler;
    taskParam->logHandler = LogHandler_CreateObj(0,taskParam->evHandler,NULL,"LogM",1);
    taskParam->paramHandler = ParamHandler_CreateObj(1,eventHandler, "Com_RX", 1); // Master handles all communication, we are master!
    taskParam->parser = SlipParser_Create(COM_PACKET_LENGTH_MAX);
    if(    !taskParam->helper || !taskParam->saveHelper
        || !taskParam->SLIP || !taskParam->evHandler  
        || !taskParam->logHandler || !taskParam->parser)
    {
        return NULL;
    }

    memset(taskParam->helper->dumpStart, 0, MAX_DEPTH);
    taskParam->helper->depth = 0;
    taskParam->helper->sequence = 0;

    memset(taskParam->saveHelper->dumpStart, 0, MAX_DEPTH);
    taskParam->saveHelper->depth = 0;
    taskParam->saveHelper->sequence = 0;

    Event_RegisterCallback(taskParam->evHandler, Msg_Log_e, Com_HandleLog, taskParam);


    return taskParam;
}

void Com_CreateTasks(eventHandler_t* eventHandlerRx, eventHandler_t* eventHandlerTx )
{
    uint8_t* receive_buffer_driver = pvPortMalloc(
            sizeof(uint8_t) * COM_PACKET_LENGTH_MAX );

    RxCom_t *paramRx = Com_InitRx(eventHandlerRx);
    TxCom_t *paramTx = Com_InitTx(eventHandlerTx);
    crcTable =  pvPortMalloc( sizeof(crc_data_t) * CRC_ARRAY_SIZE );


    crcInit();

    if(!paramTx || !paramRx || !crcTable || !receive_buffer_driver)
    {
        for ( ;; );
    }

    QuadFC_SerialOptions_t opt = {
            115200,
            EightDataBits,
            NoParity,
            OneStopBit,
            NoFlowControl,
            receive_buffer_driver,
            COM_PACKET_LENGTH_MAX
    };


    uint8_t rsp = QuadFC_SerialInit(COM_SERIAL_BUS, &opt);
    if(!rsp)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {}
    }
    rsp = QuadFC_SerialInit(COM_WIRELESS_SERIAL_BUS, &opt);
    if(!rsp)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {}
    }

    uint8_t rspSpi = SpiMaster_Init(FRAM_MEM_SPI_BUS);
    if(!rspSpi)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {}
    }
    Mem_Init();

    /* Create the freeRTOS tasks responsible for communication.*/
    portBASE_TYPE create_result_rx, create_result_tx;

    create_result_tx = xTaskCreate( Com_TxTask,           /* The task that implements the transmission of messages. */
            (const char *const) "ComTX",                  /* Name, debugging only.*/
            500,                                           /* The size of the stack allocated to the task. */
            (void *) paramTx,                                /* Pass the task parameters to the task. */
            configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
            NULL );                                        /* No handle required. */

    create_result_rx = xTaskCreate( Com_RxTask,          /* The task that implements the receiveing of messages. */
            (const char *const) "ComRX",                  /* Name, debugging only. */
            500,                                           /* The size of the stack allocated to the task. */
            (void *) paramRx,                                /* Pass the task parameters to the task. */
            configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
            NULL );                                        /* No handle required. */

    if ( create_result_tx != pdTRUE || create_result_rx != pdTRUE)
    {
        /*Error - Could not create task.*/
        for ( ;; )
        {

        }
    }

}

/**
 *
 * @param pvParameters
 */
void Com_TxTask( void *pvParameters )
{
    /**
     * The struct containing all data needed by the task.
     */
    TxCom_t * obj = (TxCom_t *) pvParameters;

    for ( ;; )
    {
        //Process incoming events.
        while(Event_Receive(obj->evHandler, 0))
        {}
        while(Event_ReceiveExternal(obj->evHandler, 0))
        {}
        vTaskDelay(1);
        // error, event receive should not fail...
    }

}
uint8_t Com_TxSend(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    TxCom_t* TxObj = (TxCom_t*)data; // The data should always be the current object when a Com_TxSend is received.

    if(TxObj->transmission != transmission_OK)
    {
        // ERROR...
    }

    if(Msg_GetType(msg) != Msg_Transmission_e)
    {
        Msg_SetMsgNr(msg, TxObj->msgNr);
    }


    uint8_t serializeBuffer[COM_PACKET_LENGTH_MAX];
    uint32_t serializebufferSize = COM_PACKET_LENGTH_MAX;
    uint8_t* buffP = Msg_Serializer(msg, serializeBuffer, serializebufferSize); // TODO change to the generated serialize function...

    uint32_t serializedLength = buffP - serializeBuffer;


    // Packetize into a slip package before writing to the serial port.
    // The buffer is allocated on the heap, and owned by the task. This allows for re-transmitt.
    result = Slip_Packetize(serializeBuffer, serializedLength, COM_PACKET_LENGTH_MAX, TxObj->txSLIP);
    if(result) // Packetization OK, write to serial port.
    {
        // Transmission is not yet OK.
        TxObj->transmission = transmission_NOK;
        uint8_t reTransmittNr = 0;
        while(TxObj->transmission == transmission_NOK)
        {
            if(Msg_Transmission_e == Msg_GetType(msg))
            {
                LOG_DBG_ENTRY(FC_SerialIOtx_e, obj, "COM TX: Sending OK/NOK nr: %lu", Msg_GetMsgNr(msg));
            }
            else
            {
                LOG_DBG_ENTRY(FC_SerialIOtx_e, obj, "COM TX: Sending message nr: %lu", Msg_GetMsgNr(msg));
            }
            /* TX is blocking */
            QuadFC_Serial_t serialData;
            serialData.buffer = TxObj->txSLIP->payload;
            serialData.bufferLength = TxObj->txSLIP->packetSize;
            result = QuadFC_SerialWrite(&serialData, COM_SERIAL_BUS, portMAX_DELAY);

            if ( !result )
            {
                //TODO error led?
                //Try retransmitt.
            }
            // We should not wait for an answer on a transmission message.
            if(Msg_GetType(msg) == Msg_Transmission_e)
            {   
                break; // TODO ugly...
            }
            if(!Event_WaitForEvent(TxObj->evHandler, Msg_Transmission_e, 1, 0, 1000)) // wait for the transmission event.
            {
                reTransmittNr ++;
                LOG_ENTRY(FC_SerialIOtx_e, obj, "COM TX: Error: Transfer failed, retransmitt. Message nr: %lu", Msg_GetMsgNr(msg));
            }
            else if(TxObj->transmission == transmission_NOK) // We got a transmission message. Check if it was a NOK message.
            {
                LOG_ENTRY(FC_SerialIOtx_e, obj, "COM TX: Timeout or NOK, retransmitt. Message nr: %lu", Msg_GetMsgNr(msg));
                reTransmittNr++; // If we got a tranmission_NOK message we should also re-transmitt.
            }
            
            if(reTransmittNr >= NR_RETRANSMITT)
            {
                LOG_ENTRY(FC_SerialIOtx_e, obj, "COM TX: Transmission failed, dropping package. Message nr: %lu", Msg_GetMsgNr(msg));
            	TxObj->transmission = transmission_OK; // We failed, and have notified this. Prepare for next message.
                TxObj->msgNr = (TxObj->msgNr+1) % 255; // wrap at 255...
                break; // Retransmit failed.
            }
        }

        Event_HandleBufferedEvents(TxObj->evHandler); // We might have buffered events, handle them if we did!
    }
    else
    {
        //TODO LED? error!
    }
    return 1;
}


void Com_RxTask( void *pvParameters )
{
    /*The already allocated data is passed to the task.*/
    RxCom_t * obj = (RxCom_t *) pvParameters;

    ParamHandler_InitMaster(obj->paramHandler);

    for ( ;; )
    {

        //Process incoming events.
        while(Event_Receive(obj->evHandler, 0) == 1)
        {
            //received event and processed it successfully! 
            (void)(obj);
        }
        /*--------------------------Receive the packet---------------------*/

        QuadFC_Serial_t serialData;
        obj->receive_buffer[0] = 0;
        serialData.buffer = obj->receive_buffer;
        serialData.bufferLength = COM_RECEIVE_BUFFER_LENGTH;

        uint32_t nr_bytes_received = QuadFC_SerialRead(&serialData, COM_SERIAL_BUS, 1);
        if(nr_bytes_received)
        {
            //LOG_DBG_ENTRY(FC_SerialIOtx_e, obj->evHandler, "Received %ld", nr_bytes_received);
        }

        SLIP_Status_t result;
        result = SlipParser_Parse(obj->parser, serialData.buffer, nr_bytes_received, obj->SLIP);
        switch ( result )
        {
        case SLIP_StatusCont:
            // Package is not finished, continue.
            break;
        case SLIP_StatusOK:
        {
            // Parse the message and send to the interested parties!
            moduleMsg_t* msg = Msg_Parse(obj->SLIP->payload,obj->SLIP->packetSize);
            if(!msg)
            {
                //TODO error
                break;
            }
            // Only relay Broadcasts to the local segment, the sender is responcible for relaying to all segments.
            if(Msg_GetDestination(msg) == BC_e)
            {
            	Msg_SetDestination(msg, GS_Broadcast_e);
            }


            if(Msg_GetType(msg) != Msg_Transmission_e) // we ack all messages except for transmission messages.
            {
                uint8_t msgNr = Msg_GetMsgNr(msg);
                moduleMsg_t* reply = Msg_TransmissionCreate(GS_SerialIO_e, msgNr, transmission_OK);
                Event_Send(obj->evHandler, reply);
            }
            if(Msg_Transmission_e == Msg_GetType(msg))
            {
                LOG_DBG_ENTRY(FC_SerialIOtx_e, obj->evHandler, "COM RX: Received OK/NOK nr: %lu", Msg_GetMsgNr(msg));
            }
            else
            {
                LOG_DBG_ENTRY(FC_SerialIOtx_e, obj->evHandler, "COM RX: Received message nr: %lu", Msg_GetMsgNr(msg));
            }
            
            Event_SendGeneric(obj->evHandler, msg);
        }
        break;
        case SLIP_StatusCrcError:
        {
            LOG_ENTRY(FC_SerialIOtx_e, obj->evHandler, "COM RX: Error - SLIP CRC error.");
        }
        case SLIP_StatusEscapeError:
        {
            LOG_ENTRY(FC_SerialIOtx_e, obj->evHandler, "COM RX: Error - SLIP escape error.");
        }
        case SLIP_StatusLengthError:
        {
            LOG_ENTRY(FC_SerialIOtx_e, obj->evHandler, "COM RX: Error - SLIP length error.");
        }
        case SLIP_StatusNok:
        {
            
            moduleMsg_t* reply = Msg_TransmissionCreate(GS_SerialIO_e, 0, transmission_NOK);
            Event_Send(obj->evHandler, reply);
        }
        break;
        default:
            break;
        }

    }
}

// This handler is called when the there is a transmission message received
uint8_t Com_TxTransmission(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    TxCom_t* handlerObj = (TxCom_t*)data; // The data should always be the handler object when a Msg_paramSave is received.

    if(handlerObj->msgNr != Msg_GetMsgNr(msg))
    {
        LOG_ENTRY(FC_SerialIOtx_e, obj, "COM: Message out of sync, expected: %d, got: %lu", handlerObj->msgNr, Msg_GetMsgNr(msg) );
    }
    else
    {
        handlerObj->msgNr = (handlerObj->msgNr+1) % 255; // wrap at 255...
        handlerObj->transmission = Msg_TransmissionGetStatus(msg); // stop transmission 
    }
    

    // This stops re-transmit.
    return 1;
}


#define LOG_MSG_REPLY_LENGTH (255)
uint8_t Com_HandleLog(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }

    RxCom_t* RxObj = (RxCom_t*)data;
    uint8_t result = 0;

    switch ( Msg_LogGetControl(msg))
    {
    case log_name:
    {
        result = 1;
        moduleMsg_t* reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_name, LOG_MSG_REPLY_LENGTH);

        LogHandler_GetNameIdMapping(RxObj->logHandler,Msg_LogGetPayload(reply), Msg_LogGetPayloadbufferlength(reply));

        uint16_t len =  strlen((char *)Msg_LogGetPayload(reply));
        Msg_LogSetPayloadlength(reply, len);

        Event_Send(obj, reply);
    }
    break;
    case log_entry:
    {
        result = 1;
        moduleMsg_t* reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_entry, LOG_MSG_REPLY_LENGTH);

        LogHandler_AppendSerializedlogs(RxObj->logHandler, Msg_LogGetPayload(reply), Msg_LogGetPayloadbufferlength(reply));
        uint16_t len =  strlen((char *)Msg_LogGetPayload(reply));
        Msg_LogSetPayloadlength(reply, len);

        Event_Send(obj, reply);
    }
    break;
    case log_stopAll:
        LogHandler_StopAllLogs(RxObj->logHandler);
        result = 0;
        break;
    default:
        break;
    }
    return result;
}