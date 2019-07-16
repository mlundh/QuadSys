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
#include "string.h"

#include "Communication/inc/communication_tasks.h"
#include "Modules/MsgBase/inc/Msg_Parser.h"
#include "Modules/Messages/inc/Msg_Transmission.h"
#include "Modules/Messages/inc/Msg_Log.h"

#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "Components/SLIP/inc/slip_packet.h"
#include "Components/SLIP/inc/crc.h"
#include "Parameters/inc/parameters.h"
#include "QuadFC/QuadFC_Memory.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "Utilities/inc/run_time_stats.h"
#include "Log/inc/logHandler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/* Use USART1 (labeled RX2 17 and TX2 16)*/
#define COM_SERIAL (1)
#define COM_RECEIVE_BUFFER_LENGTH       (2)

#define COM_PACKET_LENGTH_MAX       512

/*Global crc table*/
uint16_t *crcTable;


typedef struct RxCom
{
    QuadFC_Serial_t * serialBuffer;
    uint8_t messageBuffer[COM_PACKET_LENGTH_MAX];
    SLIP_t *SLIP;                   //!< RxTask owns SLIP package.
    uint8_t receive_buffer[COM_RECEIVE_BUFFER_LENGTH];
    param_helper_t *helper;
    param_helper_t *saveHelper;
    FlightModeHandler_t* stateHandler;
    eventHandler_t* evHandler;
    LogHandler_t* logHandler;
    uint8_t msgNr;

}RxCom_t;


typedef struct TxCom
{
    SLIP_t *txSLIP;              //!< Handle for SLIP packet, frame that will be transmitted.
    eventHandler_t* evHandler;
    uint8_t msgNr;
    TransmissionCtrl transmission;
    uint8_t serializeBuffer[COM_PACKET_LENGTH_MAX];
    uint32_t serializebufferSize;
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

//uint8_t Com_HandleDebug(eventHandler_t* obj, void* data, moduleMsg_t* msg);



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

    if( !taskParam->txSLIP || !taskParam->evHandler)
    {
        return NULL;
    }

    // Subscribe to the events that the task is interested in. All events that can be sent over the uart link.

    Event_RegisterCallback(taskParam->evHandler, Msg_Transmission_e, Com_TxTransmission, taskParam);

    Event_RegisterCallback(taskParam->evHandler, Msg_Param_e, Com_TxSend, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_Log_e, Com_TxSend, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_Debug_e, Com_TxSend, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_FlightMode_e, Com_TxSend, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_CtrlMode_e, Com_TxSend, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_FcFault_e, Com_TxSend, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_Error_e, Com_TxSend, taskParam);

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
    taskParam->helper = pvPortMalloc(sizeof(param_helper_t));
    taskParam->saveHelper = pvPortMalloc(sizeof(param_helper_t));
    taskParam->SLIP =  Slip_Create(COM_PACKET_LENGTH_MAX);
    taskParam->evHandler = eventHandler;
    taskParam->logHandler = LogHandler_CreateObj(0,taskParam->evHandler,NULL,"LogM",1);
    if( !taskParam->helper || !taskParam->saveHelper
            || !taskParam->SLIP || !taskParam->evHandler  || !taskParam->logHandler)
    {
        return NULL;
    }

    memset(taskParam->helper->dumpStart, 0, MAX_DEPTH);
    taskParam->helper->depth = 0;
    taskParam->helper->sequence = 0;

    memset(taskParam->saveHelper->dumpStart, 0, MAX_DEPTH);
    taskParam->saveHelper->depth = 0;
    taskParam->saveHelper->sequence = 0;



    // Subscribe to the events that the task is interested in. None right now.

    //Event_RegisterCallback(taskParam->evHandler, eFcFault          ,Led_HandleFcFault);

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
            57600,
            EightDataBits,
            NoParity,
            OneStopBit,
            NoFlowControl,
            receive_buffer_driver,
            COM_PACKET_LENGTH_MAX
    };


    uint8_t rsp = QuadFC_SerialInit(COM_SERIAL, &opt);
    if(!rsp)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {

        }
    }

    /* Create the freeRTOS tasks responsible for communication.*/
    portBASE_TYPE create_result_rx, create_result_tx;

    create_result_tx = xTaskCreate( Com_TxTask,           /* The task that implements the transmission of messages. */
            (const char *const) "UARTTX",                  /* Name, debugging only.*/
            500,                                           /* The size of the stack allocated to the task. */
            (void *) paramTx,                                /* Pass the task parameters to the task. */
            configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
            NULL );                                        /* No handle required. */

    create_result_rx = xTaskCreate( Com_RxTask,          /* The task that implements the receiveing of messages. */
            (const char *const) "UARTRX",                  /* Name, debugging only. */
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
        while(Event_Receive(obj->evHandler, portMAX_DELAY))
        {}
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

    Msg_SetMsgNr(msg, TxObj->msgNr);
    TxObj->msgNr = TxObj->msgNr % 255; // wrap at 255...

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
            /* TX is blocking */
            QuadFC_Serial_t serialData;
            serialData.buffer = TxObj->txSLIP->payload;
            serialData.bufferLength = TxObj->txSLIP->packetSize;
            result = QuadFC_SerialWrite(&serialData, COM_SERIAL, portMAX_DELAY);

            if ( !result )
            {
                //TODO error led?
            }

            if(!Event_WaitForEvent(TxObj->evHandler, Msg_Transmission_e, 1, 1, (1000 / portTICK_PERIOD_MS ))) // wait for the transmission event.
            {
                reTransmittNr ++;
            }
            if(reTransmittNr >= 2)
            {
                // TODO log failed retransmitt.
                break; // Retransmitt failed.
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

    int ParserIndex = 0; //!< Variable containing static information for the parser.

    /*The already allocated data is passed to the task.*/
    RxCom_t * obj = (RxCom_t *) pvParameters;

    for ( ;; )
    {

        //Process incoming events.
        while(Event_Receive(obj->evHandler, 0) == 1)
        {}
        /*--------------------------Receive the packet---------------------*/

        QuadFC_Serial_t serialData;
        serialData.buffer = obj->receive_buffer;
        serialData.bufferLength = 1;

        uint32_t nr_bytes_received = QuadFC_SerialRead(&serialData, COM_SERIAL, 1);

        SLIP_Status_t result;
        result = SLIP_Parser(obj->receive_buffer, nr_bytes_received,
                obj->SLIP, &ParserIndex);

        switch ( result )
        {
        case SLIP_StatusCont:
            // Package is not finished, continue.
            break;
        case SLIP_StatusOK:
        {
            uint32_t payloadLength = Slip_DePacketize(obj->messageBuffer, COM_PACKET_LENGTH_MAX, obj->SLIP);
            if(payloadLength == 0)
            {
                moduleMsg_t* reply = Msg_TransmissionCreate(GS_SerialIO_e, 0, transmission_NOK);
                Event_Send(obj->evHandler, reply);
                break;
            }

            // Parse the message and send to the interested parties!
            moduleMsg_t* msg = Msg_Parse(obj->messageBuffer, payloadLength);
            Event_Send(obj->evHandler, msg);

            uint8_t msgNr = Msg_GetMsgNr(msg);

            moduleMsg_t* reply = Msg_TransmissionCreate(GS_SerialIO_e, msgNr, transmission_OK);
            Event_Send(obj->evHandler, reply);

        }
        break;
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
        //Message out of sync! TODO error.
    }

    // This stops re-transmit.
    handlerObj->transmission = Msg_TransmissionGetStatus(msg);
    return 1;
}

/*
uint8_t Com_HandleDebug(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    switch ( QSP_GetControl(obj->messageBuffer) )
    {
    case QSP_DebugGetRuntimeStats:
        QuadFC_RuntimeStats(QSP_GetPayloadPtr(obj->QspRespPacket) , QSP_GetAvailibleSize(obj->QspRespPacket) - QSP_GetHeaderdSize());
        //vTaskList((char *)(QSP_GetPayloadPtr(obj->QspRespPacket)));
        uint16_t len =  strlen((char *)QSP_GetPayloadPtr(obj->QspRespPacket));
        QSP_SetPayloadSize(obj->QspRespPacket, len);
        result = 1;
        QSP_SetAddress(obj->QspRespPacket, QSP_Debug);
        QSP_SetControl(obj->QspRespPacket, QSP_DebugSetRuntimeStats);
        break;
    default:
        break;
    }

    return result;
}
 */
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
        moduleMsg_t* reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_name, LOG_MSG_REPLY_LENGTH);

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

