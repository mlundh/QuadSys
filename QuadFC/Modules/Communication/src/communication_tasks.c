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

#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "Communication/inc/slip_packet.h"
#include "Communication/inc/crc.h"
#include "Parameters/inc/parameters.h"
#include "QuadFC/QuadFC_Memory.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "Utilities/inc/run_time_stats.h"
#include "EventHandler/inc/event_handler.h"
#include "Log/inc/logHandler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/* Use USART1 (labeled RX2 17 and TX2 16)*/
#define COM_SERIAL (1)
#define COM_RECEIVE_BUFFER_LENGTH       (2)


/*Display queue*/
#define COM_QUEUE_LENGTH      (3)
#define COM_QUEUE_ITEM_SIZE         (sizeof(QSP_t*))
#define COM_PACKET_LENGTH_MAX       512
static QueueHandle_t xQueue_Com;
/*Global crc table*/
uint16_t *crcTable;


typedef struct RxCom
{
  QuadFC_Serial_t * serialBuffer;
  QSP_t *QspPacket;                  //!< RxTask owns QSP package.
  QSP_t *QspRespPacket;              //!< RxTask creates the response QSP package.
  QSP_t *QspTransRespPacket;              //!< RxTask creates the transmission response QSP package.
  SLIP_t *SLIP;                   //!< RxTask owns SLIP package.
  uint8_t *receive_buffer;
  param_helper_t *helper;
  param_helper_t *saveHelper;
  FlightModeHandler_t* stateHandler;
  eventHandler_t* evHandler;
  LogHandler_t* logHandler;

}RxCom_t;

typedef struct TxCom
{
  SLIP_t *txSLIP;              //!< Handle for SLIP packet, frame that will be transmitted.
  eventHandler_t* evHandler;
}TxCom_t;


/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
TxCom_t* Com_InitTx(QueueHandle_t eventMaster);

/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
RxCom_t* Com_InitRx(QueueHandle_t eventHandler);


/**
 * FreeRTOS task handling transmission of messages.
 * @param pvParameters
 */
static void Com_TxTask( void *pvParameters );

/**
 * FreeRTOS task handling the reception of messages.
 * @param pvParameters
 */
static void Com_RxTask( void *pvParameters );


/**
 * Save the current parameters. The two packages are cleared
 * and used internally. The QSP packet contains the result
 * of the operation, QSP_StatusAck if OK.
 */
void Com_ParamSave(RxCom_t* obj);

/**
 * Load stored parameters. The two packages are cleared
 * and used internally. The QSP packet contains the result
 * of the operation, QSP_StatusAck if OK.
 */
void Com_ParamLoad(RxCom_t* obj);

/**
 * Handle the QSP.
 * @param obj   The current object.
 * @return      0 if fail, 1 otherwise.
 */
uint8_t Com_HandleQSP(RxCom_t* obj);

/**
 * Handle a log message.
 * @param obj   The current object.
 * @return      0 if there is no response to be sent, 1 otherwise.
 */
uint8_t Com_HandleLog(RxCom_t* obj);

/**
 * Handle a parameter QSP message.
 * @param obj   The current object.
 * @return      0 if fail, 1 otherwise.
 */
uint8_t Com_HandleParameters(RxCom_t* obj);

/**
 * Handle a status QSP message.
 * @param obj   The current object.
 * @return      0 if fail, 1 otherwise.
 */
uint8_t Com_HandleStatus(RxCom_t* obj);

/**
 * Handle a Debug QSP message.
 * @param obj   The current object.
 * @return      0 if fail, 1 otherwise.
 */
uint8_t Com_HandleDebug(RxCom_t* obj);

TxCom_t* Com_InitTx(QueueHandle_t eventMaster)
{
  TxCom_t* taskParam = pvPortMalloc(sizeof(TxCom_t));
  if(!taskParam)
  {
    return NULL;
  }
  taskParam->txSLIP =  Slip_Create(COM_PACKET_LENGTH_MAX);
  xQueue_Com = xQueueCreate( COM_QUEUE_LENGTH, COM_QUEUE_ITEM_SIZE );
  taskParam->evHandler = Event_CreateHandler(eventMaster, 0);
  if( !xQueue_Com || !taskParam->txSLIP || !taskParam->evHandler)
  {
    return NULL;
  }

  // Subscribe to the events that the task is interested in. None right now.

  //Event_RegisterCallback(taskParam->evHandler, eFcFault          ,Led_HandleFcFault);
  //taskParam->evHandler->subscriptions |= 0;

  return taskParam;
}


RxCom_t* Com_InitRx(QueueHandle_t eventHandler)
{
  /* Create the queue used to pass things to the display task*/
  RxCom_t* taskParam = pvPortMalloc(sizeof(RxCom_t));
  if(!taskParam)
  {
    return NULL;
  }
  taskParam->helper = pvPortMalloc(sizeof(param_helper_t));
  taskParam->saveHelper = pvPortMalloc(sizeof(param_helper_t));
  taskParam->QspPacket = QSP_Create(QSP_MAX_PACKET_SIZE);
  taskParam->QspRespPacket = QSP_Create(QSP_MAX_PACKET_SIZE);
  taskParam->QspTransRespPacket =  QSP_Create(8); // Only header is used!
  taskParam->SLIP =  Slip_Create(COM_PACKET_LENGTH_MAX);
  taskParam->receive_buffer = pvPortMalloc(sizeof(uint8_t) * COM_RECEIVE_BUFFER_LENGTH );
  taskParam->evHandler = eventHandler;
  taskParam->logHandler = LogHandler_CreateObj(0,taskParam->evHandler,"LogM",1);
  if( !taskParam->helper || !taskParam->saveHelper || !taskParam->QspPacket
      || !taskParam->QspRespPacket || !taskParam->SLIP || !taskParam->receive_buffer
      || !taskParam->evHandler || !taskParam->QspTransRespPacket || !taskParam->logHandler)
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
  TickType_t max_block_time_ms = 333UL; //!< Max block time for transmission of messages.

  /**
   * The struct containing all data needed by the task.
   */
  TxCom_t * obj = (TxCom_t *) pvParameters;

  // Initialize event handler.
  Event_StartInitialize(obj->evHandler);

  QSP_t *txPacket;            //!< Handle for a QSP packet, used for receiving from queue.

  Event_EndInitialize(obj->evHandler);


  for ( ;; )
  {

    //Process incoming events.
    while(Event_Receive(obj->evHandler, 0) == 1)
    {}

    /* Wait blocking for items in the queue and transmit the data
      as soon as it arrives. Pack into a crc protected slip frame.*/
    if (xQueueReceive(xQueue_Com, &txPacket, portMAX_DELAY) != pdPASS )
    {
      //QSP_GiveIsEmpty(txPacket); //Should this be here?
      continue; //FrameError.
    }
    QSP_TakeIsPopulated(txPacket);
    /* We have a packet, packetize into a SLIP. */
    if(!Slip_Packetize(QSP_GetPacketPtr(txPacket), QSP_GetPacketSize(txPacket),
        QSP_GetAvailibleSize(txPacket), obj->txSLIP))
    {
      QSP_GiveIsEmpty(txPacket);
      continue; // skip this, packet not valid.
    }
    QSP_GiveIsEmpty(txPacket);
    if ( obj->txSLIP->packetSize < COM_PACKET_LENGTH_MAX )
    {
      /* TX is blocking */
      QuadFC_Serial_t serialData;
      serialData.buffer = obj->txSLIP->payload;
      serialData.bufferLength = obj->txSLIP->packetSize;
      uint8_t result = QuadFC_SerialWrite(&serialData, COM_SERIAL, max_block_time_ms);

      if ( !result )
      {
        //TODO error led?
      }
    }
    else
    {
      //TODO LED? error!
    }
  }

}

void Com_RxTask( void *pvParameters )
{

  int ParserIndex = 0; //!< Variable containing static information for the parser.

  /*The already allocated data is passed to the task.*/
  RxCom_t * obj = (RxCom_t *) pvParameters;

  // Initialize event handler.
  Event_StartInitialize(obj->evHandler);

  Event_EndInitialize(obj->evHandler);

  // TODO change timeout in mem_read and move this to the init phase.
  Com_ParamLoad(obj);
  if( QSP_StatusAck != QSP_GetControl(obj->QspRespPacket))
  {
    //No parameters to load.
  }

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

      if(Slip_DePacketize(QSP_GetPacketPtr(obj->QspPacket), QSP_GetAvailibleSize(obj->QspPacket),
          obj->SLIP))
      {
        QSP_ClearPayload(obj->QspTransRespPacket);
        QSP_SetAddress(obj->QspTransRespPacket, QSP_Transmission);
        QSP_SetControl(obj->QspTransRespPacket, QSP_TransmissionOK);
        Com_SendQSP(obj->QspTransRespPacket);

        uint8_t rxRespComplete = Com_HandleQSP(obj);
        if(rxRespComplete)
        {
          Com_SendQSP(obj->QspRespPacket);
        }
      }
      else
      {
        QSP_ClearPayload(obj->QspTransRespPacket);
        QSP_SetAddress(obj->QspTransRespPacket, QSP_Transmission);
        QSP_SetControl(obj->QspTransRespPacket, QSP_TransmissionNOK);
        Com_SendQSP(obj->QspTransRespPacket);
      }
      break;
    case SLIP_StatusNok:
      QSP_ClearPayload(obj->QspTransRespPacket);
      QSP_SetAddress(obj->QspTransRespPacket, QSP_Transmission);
      QSP_SetControl(obj->QspTransRespPacket, QSP_TransmissionNOK);
      Com_SendQSP(obj->QspTransRespPacket);
      break;
    default:
      break;
    }

  }
}


uint8_t Com_HandleQSP(RxCom_t* obj)
{
  uint8_t result = 0;
  switch ( QSP_GetAddress(obj->QspPacket) )
  {
  case QSP_Parameters:
    result = Com_HandleParameters(obj);
    break;
  case QSP_Log:
    result = Com_HandleLog(obj);
    break;
  case QSP_Status:
    result = Com_HandleStatus(obj);
    break;
  case QSP_Debug:
    result = Com_HandleDebug(obj);
    break;
  default:
    break;
  }

  return result;
}

uint8_t Com_HandleStatus(RxCom_t* obj)
{
  uint8_t result = 0;
  switch ( QSP_GetControl(obj->QspPacket) )
  {
  case QSP_StatusAck:
    break;
  case QSP_StatusCrcError:
    break;
  case QSP_StatusNack:
    break;
  case QSP_StatusNotAllowed:
    break;
  case QSP_StatusNotImplemented:
    break;
  default:
    break;
  }

  return result;
}

uint8_t Com_HandleDebug(RxCom_t* obj)
{
  uint8_t result = 0;
  switch ( QSP_GetControl(obj->QspPacket) )
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
  case QSP_DebugGetErrorMessages:
    break;
  default:
    break;
  }

  return result;
}

uint8_t Com_HandleLog(RxCom_t* obj)
{
  uint8_t result = 0;
  QSP_ClearPayload(obj->QspRespPacket);

  switch ( QSP_GetControl(obj->QspPacket) )
  {
  case QSP_LogName:
  {
    result = 1;
    LogHandler_GetNameIdMapping(obj->logHandler, QSP_GetPayloadPtr(obj->QspRespPacket), QSP_GetAvailibleSize(obj->QspRespPacket));
    uint16_t len =  strlen((char *)QSP_GetPayloadPtr(obj->QspRespPacket));
    QSP_SetPayloadSize(obj->QspRespPacket, len);
    QSP_SetAddress(obj->QspRespPacket, QSP_Log);
    QSP_SetControl(obj->QspRespPacket, QSP_LogName);
  }
  break;
  case QSP_LogEntry:
  {
    result = 1;
    LogHandler_AppendSerializedlogs(obj->logHandler, QSP_GetPayloadPtr(obj->QspRespPacket), QSP_GetAvailibleSize(obj->QspRespPacket));
    uint16_t len =  strlen((char *)QSP_GetPayloadPtr(obj->QspRespPacket));
    QSP_SetPayloadSize(obj->QspRespPacket, len);
    QSP_SetAddress(obj->QspRespPacket, QSP_Log);
    QSP_SetControl(obj->QspRespPacket, QSP_LogEntry);
  }
  break;
  case QSP_LogStopAll:
    LogHandler_StopAllLogs(obj->logHandler);
    result = 0;
    break;
  default:
    break;
  }
  return result;
}


uint8_t Com_HandleParameters(RxCom_t* obj)
{
  QSP_ClearPayload(obj->QspRespPacket);
  uint8_t result = 0;
  uint8_t sequence = obj->helper->sequence;
  switch ( QSP_GetControl(obj->QspPacket) )
  {
  case QSP_ParamGetTree:
    result = Param_DumpFromRoot(QSP_GetParamPayloadPtr(obj->QspRespPacket),
        QSP_GetAvailibleSize(obj->QspRespPacket) - QSP_GetParamHeaderdSize() - 2,
        obj->helper);
    if(result) // Dump from root is finished, reset helper.
    {
      memset(obj->helper->dumpStart, 0, MAX_DEPTH); // Starting point of dump.
      obj->helper->sequence = 0;
      QSP_SetParamLastInSeq(obj->QspRespPacket, 1); // This is the last in the sequence.
    } // else dump is not finished, keep helper till next time.
    else
    {
      QSP_SetParamLastInSeq(obj->QspRespPacket, 0);
      obj->helper->sequence++;
    }


    // Always send dump.
    uint16_t len =  strlen((char *)QSP_GetParamPayloadPtr(obj->QspRespPacket));
    if(len <= (QSP_MAX_PACKET_SIZE - QSP_GetParamHeaderdSize()))
    {
      //TODO add return value checks!
      QSP_SetParamSequenceNumber(obj->QspRespPacket, sequence);
      QSP_SetParamPayloadSize(obj->QspRespPacket, len);
      QSP_SetAddress(obj->QspRespPacket, QSP_Parameters);
      QSP_SetControl(obj->QspRespPacket, QSP_ParamSetTree);
    }
    else
    {
      QSP_ClearPayload(obj->QspRespPacket);
      QSP_SetAddress(obj->QspRespPacket, QSP_Status);
      QSP_SetControl(obj->QspRespPacket, QSP_StatusBufferOverrun);
    }

    break;
  case QSP_ParamSetTree:
    result = Param_SetFromHere(Param_GetRoot(), QSP_GetParamPayloadPtr(obj->QspPacket),
        QSP_GetParamPayloadSize(obj->QspPacket));
    QSP_SetAddress(obj->QspRespPacket, QSP_Status);
    if(result) // Set was ok, respond with Ack.
    {
      QSP_SetControl(obj->QspRespPacket, QSP_StatusAck);
    }
    else  // Set was not ok, respond with Nack.
    {
      QSP_SetControl(obj->QspRespPacket, QSP_StatusNack);
    }
    break;
  case QSP_ParamSave:
    Com_ParamSave(obj);
    break;
  case QSP_ParamLoad:
    Com_ParamLoad(obj);
    break;
  default:
    break;
  }

  return 1; // Param messages always require an answer.
}


uint8_t Com_SendQSP( QSP_t *packet )
{
  QSP_GiveIsPopulated(packet);
  BaseType_t result = xQueueSendToBack( xQueue_Com, &packet, mainDONT_BLOCK );
  QSP_TakeIsEmpty(packet);
  if(result != pdTRUE)
  {
    return 0;
  }
  return 1;
}

void Com_ParamSave(RxCom_t* obj)
{
  uint8_t cont = 1;
  uint32_t address = 0;
  memset(obj->saveHelper->dumpStart, 0, MAX_DEPTH);
  obj->saveHelper->depth = 0;
  obj->saveHelper->sequence = 0;
  while(cont)
  {
    uint8_t result = 0;
    QSP_ClearPayload(obj->QspPacket);
    uint8_t sequence = obj->saveHelper->sequence;
    result = Param_DumpFromRoot(QSP_GetParamPayloadPtr(obj->QspPacket),
        QSP_GetAvailibleSize(obj->QspPacket) - QSP_GetParamHeaderdSize(), obj->saveHelper);

    if(result) // Dump from root is finished, reset helper.
    {
      memset(obj->saveHelper->dumpStart, 0, MAX_DEPTH);          // Starting point of dump.
      obj->saveHelper->sequence = 0;                             // Next will be first in sequence!
      QSP_SetParamLastInSeq(obj->QspPacket, 1);             // This is the last in the sequence.
      cont = 0;                                         // Last package, nothing more to save.
    } // dump is not finished, keep helper till next time.
    else
    {
      QSP_SetParamLastInSeq(obj->QspPacket, 0);
      obj->saveHelper->sequence++;
    }
    // Always save dump.
    uint16_t len =  strlen((char *)QSP_GetParamPayloadPtr(obj->QspPacket));
    result = (len <= (QSP_MAX_PACKET_SIZE - QSP_GetParamHeaderdSize()));
    if(result)
    {
      // Save!
      result &= QSP_SetParamSequenceNumber(obj->QspPacket, sequence);
      result &= QSP_SetParamPayloadSize(obj->QspPacket,len);
      result &= QSP_SetAddress(obj->QspPacket, QSP_Parameters);
      result &= QSP_SetControl(obj->QspPacket, QSP_ParamSetTree);
    }
    if(result)
    {

      // Packetize into a slip package before write to mem.
      result = Slip_Packetize(QSP_GetPacketPtr(obj->QspPacket),
          QSP_GetPacketSize(obj->QspPacket), QSP_GetAvailibleSize(obj->QspPacket),
          obj->SLIP);
    }
    if(result) // Packetization OK, write to mem.
    {
      result = Mem_Write(address, obj->SLIP->packetSize,
          obj->SLIP->payload, obj->SLIP->allocatedSize);
      address += (obj->SLIP->packetSize + 1);
    }
    if(result) // Mem write was OK, send ack to indicate OK save of params.
    {
      result &= QSP_ClearPayload(obj->QspRespPacket);
      result &= QSP_SetAddress(obj->QspRespPacket, QSP_Status);
      result &= QSP_SetControl(obj->QspRespPacket, QSP_StatusAck);
    }
    else // Packetization or mem_Write did not succeed.
    {
      cont = 0;
      QSP_ClearPayload(obj->QspRespPacket);
      QSP_SetAddress(obj->QspRespPacket, QSP_Status);
      QSP_SetControl(obj->QspRespPacket, QSP_StatusNack);
    }

  }// while(cont)
}

void Com_ParamLoad(RxCom_t* obj)
{
  uint8_t lastInSequence = 0;
  uint8_t EcpectedSequenceNo = 0;
  uint32_t address = 0;

  while(!lastInSequence)
  {
    QSP_ClearPayload(obj->QspPacket);
    int k = 0;
    uint8_t buffer[2];
    SLIP_Status_t read_status = SLIP_StatusCont;
    uint8_t result = 0;
    uint32_t startAddress = address;
    // Read from memory and add to the parser.
    while((SLIP_StatusCont == read_status))
    {
      result = Mem_Read(address, 1, buffer, 2);
      if(result && ((address - startAddress) < COM_PACKET_LENGTH_MAX))
      {
        read_status = SLIP_Parser(buffer, 1, obj->SLIP, &k);
      }
      else
      {
        result = 0;
        break;
      }
      address++;
    }
    result &= (read_status == SLIP_StatusOK);
    if(result){
      if(!Slip_DePacketize(QSP_GetPacketPtr(obj->QspPacket), QSP_GetAvailibleSize(obj->QspPacket),
          obj->SLIP))
      {
        result = 0;
      }
    }
    if(result)
    {
      uint8_t sequenceNo = QSP_GetParamSequenceNumber(obj->QspPacket);
      lastInSequence = QSP_GetParamLastInSeq(obj->QspPacket);
      result = (sequenceNo == EcpectedSequenceNo);
      EcpectedSequenceNo++;
    }
    if(result)
    {
      result = Param_SetFromHere(Param_GetRoot(),
          QSP_GetParamPayloadPtr(obj->QspPacket),
          QSP_GetParamPayloadSize(obj->QspPacket));
    }
    if(result) // Set was successful, params are loaded from mem.
    {
      QSP_ClearPayload(obj->QspRespPacket);
      QSP_SetAddress(obj->QspRespPacket, QSP_Status);
      QSP_SetControl(obj->QspRespPacket, QSP_StatusAck);
    }
    else // Read or set was not successful, send Nack.
    {
      QSP_ClearPayload(obj->QspRespPacket);
      QSP_SetAddress(obj->QspRespPacket, QSP_Status);
      QSP_SetControl(obj->QspRespPacket, QSP_StatusNack);
      lastInSequence = 1;
    }
  }
}

