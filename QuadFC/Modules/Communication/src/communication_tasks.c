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
#include "Messages/inc/Msg_Parser.h"
#include "Messages/inc/Msg_Transmission.h"
#include "Messages/inc/Msg_ChangeComPort.h"
#include "BoardConfig.h"
#include "Components/Parameters/inc/paramHandler.h"



#include "Components/SLIP/inc/slip_packet.h"
#include "Components/SLIP/inc/crc.h"
#include "Components/AppLog/inc/AppLogHandler.h"
#include "Components/AppLog/inc/AppLog.h"
#include "QuadFC/QuadFC_Serial.h"
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
    eventHandler_t* evHandler;
    uint32_t uartNrUSB;
    uint32_t uartNrWirless;
    uint32_t busNr;
    uint32_t paramBusNr;
    uint32_t paramPrevBusNr;
    paramHander_t* paramHandler;
}RxCom_t;


typedef struct TxCom
{
    SLIP_t *txSLIP;              //!< Handle for SLIP packet, frame that will be transmitted.
    eventHandler_t* evHandler;
    uint8_t msgNr;
    TransmissionCtrl transmission;
    uint8_t serializeBuffer[COM_PACKET_LENGTH_MAX];
    uint32_t serializebufferSize;
    uint32_t uartNrUSB;
    uint32_t uartNrWirless;
    uint32_t busNr;
}TxCom_t;


/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
TxCom_t* Com_InitTx(eventHandler_t* eventHandler, uint32_t uartNrUSB, uint32_t uartNrWirless);

/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
RxCom_t* Com_InitRx(eventHandler_t* eventHandler, uint32_t uartNrUSB, uint32_t uartNrWirless);


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

uint8_t Com_TxChangePort(eventHandler_t* obj, void* data, moduleMsg_t* msg);


TxCom_t* Com_InitTx(eventHandler_t* eventHandler, uint32_t uartNrUSB, uint32_t uartNrWirless)
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
    taskParam->uartNrUSB = uartNrUSB;
    taskParam->uartNrWirless = uartNrWirless;
    taskParam->busNr = uartNrUSB;

    if( !taskParam->txSLIP || !taskParam->evHandler)
    {
        return NULL;
    }

    // Subscribe to the events that the task is interested in. All events that can be sent over the uart link.

    Event_RegisterPortFunction(taskParam->evHandler, Com_TxSend, taskParam);

    Event_RegisterCallback(taskParam->evHandler, Msg_Transmission_e, Com_TxTransmission, taskParam);
    Event_RegisterCallback(taskParam->evHandler, Msg_ChangeComPort_e, Com_TxChangePort, taskParam);

    return taskParam;
}


RxCom_t* Com_InitRx(eventHandler_t* eventHandler, uint32_t uartNrUSB, uint32_t uartNrWirless)
{
    /* Create the queue used to pass things to the display task*/
    RxCom_t* taskParam = pvPortMalloc(sizeof(RxCom_t));
    if(!taskParam)
    {
        return NULL;
    }

    taskParam->SLIP =  Slip_Create(COM_PACKET_LENGTH_MAX);
    taskParam->evHandler = eventHandler;
    taskParam->parser = SlipParser_Create(COM_PACKET_LENGTH_MAX);
    taskParam->uartNrUSB = uartNrUSB;
    taskParam->uartNrWirless = uartNrWirless;
    taskParam->busNr = uartNrUSB;
    taskParam->paramBusNr = 0;
    taskParam->paramPrevBusNr = taskParam->paramBusNr;
    taskParam->paramHandler = ParamHandler_CreateObj(1,eventHandler, "ComRx");
    Param_CreateObj(0, variable_type_uint32, readWrite, &taskParam->paramBusNr, "UartBus", ParamHandler_GetParam(taskParam->paramHandler));

    if(!taskParam->SLIP || !taskParam->evHandler  || !taskParam->parser)
    {
        return NULL;
    }

    return taskParam;
}

void Com_CreateTasks(eventHandler_t* eventHandlerRx, eventHandler_t* eventHandlerTx, uint32_t uartNrUSB, uint32_t uartNrWirless)
{
    uint8_t* receive_buffer_driver = pvPortMalloc(
            sizeof(uint8_t) * COM_PACKET_LENGTH_MAX );

    RxCom_t *paramRx = Com_InitRx(eventHandlerRx, uartNrUSB, uartNrWirless);
    TxCom_t *paramTx = Com_InitTx(eventHandlerTx, uartNrUSB, uartNrWirless);
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


    uint8_t rsp = QuadFC_SerialInit(uartNrUSB, &opt);
    if(!rsp)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {}
    }


    QuadFC_SerialOptions_t optWl = {
            57600,
            EightDataBits,
            NoParity,
            OneStopBit,
            NoFlowControl,
            receive_buffer_driver,
            COM_PACKET_LENGTH_MAX
    };


    rsp = QuadFC_SerialInit(uartNrWirless, &optWl);
    if(!rsp)
    {
        /*Error - Could not create serial interface.*/

        for ( ;; )
        {}
    }


    /* Create the freeRTOS tasks responsible for communication.*/
    portBASE_TYPE create_result_rx, create_result_tx;

    create_result_tx = xTaskCreate( Com_TxTask,            /* The task that implements the transmission of messages. */
            (const char *const)"ComTx",                    /* Name, debugging only.*/
            500,                                           /* The size of the stack allocated to the task. */
            (void *) paramTx,                              /* Pass the task parameters to the task. */
            configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
            NULL );                                        /* No handle required. */

    create_result_rx = xTaskCreate( Com_RxTask,             /* The task that implements the receiveing of messages. */
            (const char *const)"ComRx",                     /* Name, debugging only. */
            500,                                            /* The size of the stack allocated to the task. */
            (void *) paramRx,                               /* Pass the task parameters to the task. */
            configMAX_PRIORITIES-3,                         /* The priority allocated to the task. */
            NULL );                                         /* No handle required. */

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
        TxObj->msgNr = (TxObj->msgNr+1) % 255;
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
                LOG_DBG_ENTRY( obj, "COM TX: Sending OK/NOK nr: %lu", Msg_GetMsgNr(msg));
            }
            else
            {
                LOG_DBG_ENTRY( obj, "COM TX: Sending message nr: %lu", Msg_GetMsgNr(msg));
            }
            /* TX is blocking */
            QuadFC_Serial_t serialData;
            serialData.buffer = TxObj->txSLIP->payload;
            serialData.bufferLength = TxObj->txSLIP->packetSize;
            result = QuadFC_SerialWrite(&serialData, TxObj->busNr, portMAX_DELAY);

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
                LOG_ENTRY(obj, "COM TX: Timeout: Transmission msg not received, retransmitt. Message nr: %lu", Msg_GetMsgNr(msg));
            }
            if(TxObj->transmission == transmission_NOK) // We got a transmission message. Check if it was a NOK message.
            {
                LOG_ENTRY(obj, "COM TX: Timeout or NOK, retransmitt. Message nr: %lu", Msg_GetMsgNr(msg));
                reTransmittNr++; // If we got a tranmission_NOK message we should also re-transmitt.

                if(reTransmittNr >= NR_RETRANSMITT)
                {
                    LOG_ENTRY(obj, "COM TX: Transmission failed, dropping package. Message nr: %lu", Msg_GetMsgNr(msg));
                	TxObj->transmission = transmission_OK; // We failed, and have notified this. Prepare for next message.
                    break; // Retransmit failed.
                }
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

    for ( ;; )
    {
        //Process incoming events.
        while(Event_Receive(obj->evHandler, 0) == 1)
        {
            //received event and processed it successfully! 
            (void)(obj);
        }
        if(obj->paramBusNr != obj->paramPrevBusNr) // If the parameter has changed then make sure it has the correct bus number.
        {
            if(obj->paramBusNr == 0)
            {
                obj->busNr = obj->uartNrUSB;
            }
            else
            {
                obj->busNr = obj->uartNrWirless;
            }
            obj->paramPrevBusNr = obj->paramBusNr;
            moduleMsg_t* msg = Msg_ChangeComPortCreate(FC_Broadcast_e, 0, obj->busNr);
            Event_Send(obj->evHandler, msg);
        }

        /*--------------------------Receive the packet---------------------*/

        QuadFC_Serial_t serialData;
        obj->receive_buffer[0] = 0;
        serialData.buffer = obj->receive_buffer;
        serialData.bufferLength = COM_RECEIVE_BUFFER_LENGTH;

        uint32_t nr_bytes_received = QuadFC_SerialRead(&serialData, obj->busNr, 1);
        if(nr_bytes_received)
        {
            //LOG_DBG_ENTRY( obj->evHandler, "Received %ld", nr_bytes_received);
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
                LOG_DBG_ENTRY( obj->evHandler, "COM RX: Received OK/NOK nr: %lu", Msg_GetMsgNr(msg));
            }
            else
            {
                LOG_DBG_ENTRY( obj->evHandler, "COM RX: Received message nr: %lu", Msg_GetMsgNr(msg));
            }
            
            Event_SendGeneric(obj->evHandler, msg);
        }
        break;
        case SLIP_StatusCrcError:
        {
            LOG_ENTRY(obj->evHandler, "COM RX: Error - SLIP CRC error.");
        }
        case SLIP_StatusEscapeError:
        {
            LOG_ENTRY(obj->evHandler, "COM RX: Error - SLIP escape error.");
        }
        case SLIP_StatusLengthError:
        {
            LOG_ENTRY(obj->evHandler, "COM RX: Error - SLIP length error.");
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
        LOG_ENTRY(obj, "COM: Message out of sync, expected: %d, got: %lu, re-transmitt!", handlerObj->msgNr, Msg_GetMsgNr(msg) );
    }
    else
    {
        handlerObj->transmission = Msg_TransmissionGetStatus(msg); // stop transmission
    }
    return 1;
}

uint8_t Com_TxChangePort(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    TxCom_t* handlerObj = (TxCom_t*)data; // The data should always be the handler object when a Msg_paramSave is received.
    uint32_t uartNr = Msg_ChangeComPortGetUartnr(msg);
    if(uartNr == handlerObj->uartNrWirless)
    {
        handlerObj->busNr = handlerObj->uartNrWirless;
    }
    else
    {
        handlerObj->busNr = handlerObj->uartNrUSB;
    }
    return 1;
}