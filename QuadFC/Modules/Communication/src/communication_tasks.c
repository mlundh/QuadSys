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

#include "communication_tasks.h"
#include "parameters.h"
#include "QuadFC_Memory.h"
#include "led_control_task.h"
#include "globals.h"
#include "crc.h"
#include "slip_packet.h"
#include "freertos_uart_serial.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"



#include <gpio.h>
#include <pio.h>

/* Use USART1 (labeled RX2 17 and TX2 16)*/
#define TERMINAL_USART USART1

/*Display queue*/
#define COM_QUEUE_LENGTH      (1)
#define COM_QUEUE_ITEM_SIZE         (sizeof(QSP_t*))
static QueueHandle_t xQueue_Com;
#define COM_PACKET_LENGTH_MAX       512

/**
 * Initialize the communication module.
 * @return    1 if OK, 0 otherwise.
 */
uint8_t Com_Init();

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
 * Parser that builds a SLIP package via multiple calls. The
 * parser function will return with QSP_StatusCont if no packet
 * is finished, and QSP_StatusAck if there is a package. Any
 * other response is an error condition.
 *
 * The index is used internally to keep track of index in the
 * raw slip packet. User has to provide this to ensure thread
 * safety. User should not update except for discarding data
 * already saved in the SLIP packet.
 * @param buffer          Buffer containing > 1 bytes of data.
 * @param buffer_length   number of bytes in buffer.
 * @param SLIP_packet     Slip packet used internally.
 * @param QSP_packet      Resulting QSP.
 * @param index           User should initialize to zero and then not update.
 * @return                QSP_StatusAck when done, QSP_StatusCont if not done, anything else is an error condition.
 */
QSP_StatusControl_t SLIP_QspParser(uint8_t *buffer,
    int buffer_length,
    SLIP_t *SLIP_packet,
    QSP_t *QSP_packet,
    int *index);

/**
 * Save the current parameters. The two packages are cleared
 * and used internally. The QSP packet contains the result
 * of the operation, QSP_StatusAck if OK.
 * @param SLIP_packet       QSP with response.
 * @param QSP_Packet        SLIP used internally.
 */
void Param_Save(SLIP_t *SLIP_packet, QSP_t *QSP_Packet);

/**
 * Load stored parameters. The two packages are cleared
 * and used internally. The QSP packet contains the result
 * of the operation, QSP_StatusAck if OK.
 * @param SLIP_packet       QSP with response.
 * @param QSP_Packet        SLIP used internally.
 */
void Param_Load(SLIP_t *SLIP_packet, QSP_t *QSP_Packet);




uint8_t Com_Init()
{
  /* Create the queue used to pass things to the display task*/
  xQueue_Com = xQueueCreate( COM_QUEUE_LENGTH, COM_QUEUE_ITEM_SIZE );
  crcTable = pvPortMalloc( sizeof(uint16_t) * 256 );
  if( !xQueue_Com ||  !crcTable )
  {
    return 0;
  }
  crcInit();
  return 1;
}

void Com_CreateTasks( void )
{


  //TODO move all init of peripheral to board init.
  if(!Com_Init())
  {
    for ( ;; );
  }
  uint8_t* receive_buffer_driver = pvPortMalloc(
      sizeof(uint8_t) * COM_PACKET_LENGTH_MAX );

  freertos_usart_if freertos_usart;

  freertos_peripheral_options_t driver_options = { receive_buffer_driver,   /* The buffer used internally by the USART driver to store incoming characters. */
      COM_PACKET_LENGTH_MAX,                                                /* The size of the buffer provided to the USART driver to store incoming characters. */
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4),                   /* The priority used by the USART interrupts. */
      USART_RS232,                                                          /* Configure the USART for RS232 operation. */
      (WAIT_TX_COMPLETE)                                                    /* Wait for a Tx to complete before returning from send functions. */
  };

  const sam_usart_opt_t usart_settings = { 57600,                           /* Speed of the transfer, baud*/
      US_MR_CHRL_8_BIT,                                                     /* 8 bit transfer*/
      US_MR_PAR_NO,                                                         /* No parity*/
      US_MR_NBSTOP_1_BIT,                                                   /* One stop bit*/
      US_MR_CHMODE_NORMAL,                                                  /* */
      0                                                                     /* Only used in IrDA mode. */
  };

  /* Initialize the USART interface. TODO move this. */
  freertos_usart = freertos_usart_serial_init( TERMINAL_USART, &usart_settings,
      &driver_options );


  /**
   * Create the freeRTOS tasks responsible for
   */
  xTaskCreate( Com_TxTask,                           /* The task that implements the transmission of messages. */
      (const char *const) "UARTTX",                  /* Name, debugging only.*/
      500,                                           /* The size of the stack allocated to the task. */
      (void *) freertos_usart,                       /* Pass the already configured USART port into the task. */
      configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
      NULL );                                        /* No handle required. */

  xTaskCreate( Com_RxTask,                           /* The task that implements the receiveing of messages. */
      (const char *const) "UARTRX",                  /* Name, debugging only. */
      500,                                           /* The size of the stack allocated to the task. */
      (void *) freertos_usart,                       /* Pass the already configured USART port into the task. */
      configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
      NULL );                                        /* No handle required. */
}

/**
 *
 * @param pvParameters
 */
void Com_TxTask( void *pvParameters )
{
  TickType_t max_block_time_ticks = 333UL / portTICK_PERIOD_MS; //!< Max block time for transmission of messages.

  status_code_t result;

  QSP_t *txPacket;                                      //!< Handle for a QSP packet, used for receiving from queue.
  SLIP_t *txSLIP = Slip_Create(COM_PACKET_LENGTH_MAX);  //!< Handle for SLIP packet, frame that will be transmitted.

  /**
   * The already open usart port is passed through the task parameters TODO fix peripherals.
   */
  freertos_usart_if xbee_usart = (freertos_usart_if) pvParameters;

  for ( ;; )
  {

    /**
     * Wait blocking for items in the queue and transmit the data
     * as soon as it arrives. Pack into a crc protected slip frame.
     */
    if (xQueueReceive(xQueue_Com, &txPacket, portMAX_DELAY) != pdPASS )
    {
      continue; //FrameError.
    }

    /**
     * We have a packet, calculate and append crc. Packetize.
     */
    Com_AddCRC(txPacket);
    if(!Slip_Packetize(QSP_GetPacketPtr(txPacket), QSP_GetPayloadSize(txPacket) + QSP_GetHeaderdSize(txPacket) + 2, txSLIP))
    {
      continue; // skip this, packet not valid.
    }
    if ( txSLIP->packetSize < COM_PACKET_LENGTH_MAX )
    {

      // TX is blocking
      result = freertos_usart_write_packet( xbee_usart,
          txSLIP->payload,
          txSLIP->packetSize,
          max_block_time_ticks );

      if ( result != STATUS_OK )
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

  int k = 0;

  uint8_t *receive_buffer = pvPortMalloc(sizeof(uint8_t) * 2 );

  QSP_t *rxPacket = QSP_Create(QSP_MAX_PACKET_SIZE);      //!< RxTask owns QSP package.
  QSP_t *rxRespPacket = QSP_Create(QSP_MAX_PACKET_SIZE);      //!< RxTask creates the response QSP package. Gives ownership to tx.
  SLIP_t *rxSLIP = Slip_Create(COM_PACKET_LENGTH_MAX);    //!< RxTask owns SLIP package.

  if(!rxPacket || !rxSLIP)
  {
    //ERROR! TODO
    for ( ;; );
  }
  Param_Load(rxSLIP, rxRespPacket);
  if( QSP_StatusAck != QSP_GetControl(rxRespPacket))
  {
    //No parameters to load.
  }

  /*The already open usart port is passed through the task parameters*/
  freertos_usart_if xbee_usart = (freertos_usart_if) pvParameters;

  for ( ;; )
  {

    /*--------------------------Receive the packet---------------------*/

    uint32_t nr_bytes_received;
    nr_bytes_received = freertos_usart_serial_read_packet( xbee_usart,  /* The USART port*/
        receive_buffer,                                                 /* Pointer to the buffer into which received data is to be copied.*/
        1,                                                              /* The number of bytes to copy. */
        portMAX_DELAY );                                                /* Block time*/
    QSP_StatusControl_t result;
    result = SLIP_QspParser(receive_buffer,
        nr_bytes_received,
        rxSLIP,
        rxPacket,
        &k);
    uint8_t answer_qsp;
    switch ( result )
    {
    case QSP_StatusCont:
      // Package is not finished, continue.
      break;
    case QSP_StatusAck:
      answer_qsp = Com_HandleQSP(rxPacket, rxRespPacket, rxSLIP);
      if(answer_qsp)
      {
        Com_SendQSP(rxRespPacket);
      }
      break;
    case QSP_StatusNack: /* FALLTHROUGH */
    case QSP_StatusCrcError:
    case QSP_StatusNotAllowed:
    case QSP_StatusNotImplemented:
      QSP_ClearPayload(rxRespPacket);
      QSP_SetAddress(rxRespPacket, QSP_Status);
      QSP_SetControl(rxRespPacket, result);
      Com_SendQSP(rxRespPacket);
      break;
    default:
      break;
    }

  }
}


QSP_StatusControl_t SLIP_QspParser(uint8_t *buffer,
    int buffer_length,
    SLIP_t *SLIP_packet,
    QSP_t *QSP_packet,
    int *index)
{
  if ( buffer_length < 1 )
  {
    return 0;
  }
  QSP_StatusControl_t result = QSP_StatusCont;
  for ( int i = 0; i < buffer_length; i++, (*index)++ )
  {
    SLIP_packet->payload[*index] = buffer[i];
    if ( SLIP_packet->payload[*index] == frame_boundary_octet )
    {
      if ( *index > 4 ) // Last boundary of frame. Frame minimum length = 4
      {
        SLIP_packet->packetSize = *index + 1;
        *index = 0;
        /*handle communication*/
        if(Slip_DePacketize(QSP_GetPacketPtr(QSP_packet),
            QSP_GetAvailibleSize(QSP_packet),
            SLIP_packet))
        {
          if(Com_CheckCRC(QSP_packet)) // CRC ok!
          {
            result = QSP_StatusAck;
          }
          else // CRC error.
          {
            result = QSP_StatusCrcError;
          }
        }
        else // Not valid slip packet.
        {
          result =  QSP_StatusNotValidSlipPacket;
        }
      }
      else // First boundary of frame.
      {
        *index = 0;
        SLIP_packet->payload[*index] = buffer[i];
      }
    }// Frame boundary
  }//for()
  return result;
}


uint8_t Com_HandleQSP( QSP_t *QSP_packet, QSP_t *QSP_RspPacket, SLIP_t *SLIP_packet)
{
  uint8_t result = 0;
  switch ( QSP_GetAddress(QSP_packet) )
  {
  case QSP_Parameters:
    result = Com_HandleParameters(QSP_packet, QSP_RspPacket, SLIP_packet);
    break;
  case QSP_Status:
    result = Com_HandleStatus(QSP_packet, QSP_RspPacket, SLIP_packet);
    break;
  default:
    break;
  }

  return result;
}

uint8_t Com_HandleStatus(QSP_t *QSP_packet, QSP_t *QSP_RspPacket, SLIP_t *SLIP_packet)
{
  uint8_t result = 0;
  switch ( QSP_GetControl(QSP_packet) )
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
uint8_t Com_HandleParameters(QSP_t *QSP_packet, QSP_t *QSP_RspPacket, SLIP_t *SLIP_packet)
{
  QSP_ClearPayload(QSP_RspPacket);
  uint8_t result = 0;
  switch ( QSP_GetControl(QSP_packet) )
  {
  case QSP_ParamGetTree:
    result = Param_DumpFromRoot(Param_GetRoot(), QSP_GetPayloadPtr(QSP_RspPacket), QSP_GetAvailibleSize(QSP_RspPacket) - QSP_GetHeaderdSize());
    if(result) // Dump from root was OK, create packet.
    {
      uint16_t len =  strlen((char *)QSP_GetPayloadPtr(QSP_RspPacket));
      if(len <= QSP_MAX_PACKET_SIZE)
      {
        QSP_SetPayloadSize(QSP_RspPacket,len);
        QSP_SetAddress(QSP_RspPacket, QSP_Parameters);
        QSP_SetControl(QSP_RspPacket, QSP_ParamSetTree);
      }
      else
      {
        QSP_ClearPayload(QSP_RspPacket);
        QSP_SetAddress(QSP_RspPacket, QSP_Status);
        QSP_SetControl(QSP_RspPacket, QSP_StatusBufferOverrun);
      }
    }
    else // Dump was not ok, update status with Nack status.
    {
      QSP_ClearPayload(QSP_RspPacket);
      QSP_SetAddress(QSP_RspPacket, QSP_Status);
      QSP_SetControl(QSP_RspPacket, QSP_StatusNack);
    }
    break;
  case QSP_ParamSetTree:
    result = Param_SetFromRoot(Param_GetRoot(),
        QSP_GetPayloadPtr(QSP_packet),
        QSP_GetPayloadSize(QSP_packet));
    QSP_SetAddress(QSP_RspPacket, QSP_Status);
    if(result) // Set was ok, respond with Ack.
    {
      QSP_SetControl(QSP_RspPacket, QSP_StatusAck);
    }
    else  // Set was not ok, respond with Nack.
    {
      QSP_SetControl(QSP_RspPacket, QSP_StatusNack);
    }
    break;
  case QSP_ParamSave: // (miss)use the response and slip packages to save data in memory.
    Param_Save(SLIP_packet, QSP_RspPacket);
    break;
  case QSP_ParamLoad: // (miss)use the response and slip packages to load data from memory.
    Param_Load(SLIP_packet, QSP_RspPacket);
    break;
  default:
    break;
  }

  return 1; // Param messages always require an answer.
}


uint8_t Com_SendQSP( QSP_t *packet )
{
  //TODO add check to make sure sending was successful.
  //QSP_GiveOwnership(packet);
  status_code_t result = xQueueSendToBack( xQueue_Com, &packet, mainDONT_BLOCK );
  if(result != STATUS_OK)
  {
    return 0;
  }
  return 1;
}


uint8_t Com_AddCRC( QSP_t *packet)
{

  //Calculate and append CRC16 checksum.
  uint16_t crcCalc;
  crcCalc = crcFast( QSP_GetPacketPtr(packet), (QSP_GetPayloadSize(packet) + QSP_GetHeaderdSize()));
  uint8_t tmp[2];
  tmp[0] = (uint8_t) ((crcCalc >> 8) & 0Xff);
  tmp[1] = (uint8_t) ((crcCalc) & 0Xff);
  if(!QSP_SetAfterPayload(packet, tmp, 2))
  {
    return 0;
  }
  return 1;
}

uint8_t Com_CheckCRC( QSP_t *packet)
{
  uint16_t payloadSize = QSP_GetPayloadSize(packet);

  //Calculate and compare CRC16 checksum.
  uint16_t crcCalc = 0;
  uint16_t crcMsg = 0;
  crcCalc = crcFast( QSP_GetPacketPtr(packet), (payloadSize + QSP_GetHeaderdSize()));

  crcMsg |=  (uint16_t)(QSP_GetPacketPtr(packet)[QSP_GetHeaderdSize() + payloadSize]) << 8;
  crcMsg |=  (uint16_t)(QSP_GetPacketPtr(packet)[QSP_GetHeaderdSize() + payloadSize + 1]);

  if ( crcCalc != crcMsg )
  {
    return 0;
  }
  return 1;
}

void Param_Save(SLIP_t *SLIP_packet, QSP_t *QSP_Packet)
{
  uint8_t result = 0;
  result = Param_DumpFromRoot(Param_GetRoot(),
      QSP_GetPayloadPtr(QSP_Packet),
      QSP_GetAvailibleSize(QSP_Packet) - QSP_GetHeaderdSize());
  if(result) // Dump was OK, packetize the QSP into a slip packet.
  {
    QSP_SetPayloadSize(QSP_Packet, strlen((char *)QSP_GetPayloadPtr(QSP_Packet)));
    QSP_SetAddress(QSP_Packet, QSP_Parameters);
    QSP_SetControl(QSP_Packet, QSP_ParamSetTree);
    Com_AddCRC(QSP_Packet);

    result = Slip_Packetize(QSP_GetPacketPtr(QSP_Packet),
        QSP_GetPayloadSize(QSP_Packet) + QSP_GetHeaderdSize(QSP_Packet) + 2,
        SLIP_packet);
    if(result) // Packetization OK, write to mem.
    {
      result = Mem_Write(0,
          SLIP_packet->packetSize,
          SLIP_packet->payload,
          SLIP_packet->allocatedSize);
      if(result) // Mem write was OK, send ack to indicate OK save of params.
      {
        QSP_ClearPayload(QSP_Packet);
        QSP_SetAddress(QSP_Packet, QSP_Status);
        QSP_SetControl(QSP_Packet, QSP_StatusAck);
      }
    }

  }
  if(!result)
  {
    QSP_ClearPayload(QSP_Packet);
    QSP_SetAddress(QSP_Packet, QSP_Status);
    QSP_SetControl(QSP_Packet, QSP_StatusNack);
  }
}

void Param_Load(SLIP_t *SLIP_packet, QSP_t *QSP_Packet)
{
  int k = 0;
  uint8_t buffer[2];
  QSP_StatusControl_t read_status = QSP_StatusCont;
  uint32_t counter = 0;
  uint8_t result = 0;
  while(QSP_StatusCont == read_status && counter < QSP_MAX_PACKET_SIZE)
  {
    result = Mem_Read(counter,1, buffer, 2);
    counter++;
    if(result)
    {
      read_status = SLIP_QspParser(buffer,
          1,
          SLIP_packet,
          QSP_Packet,
          &k);
    }
    else
    {
      result = 0;
      break;
    }
  }
  if(result)
  {
    result = Param_SetFromRoot(Param_GetRoot(),
        QSP_GetPayloadPtr(QSP_Packet),
        QSP_GetPayloadSize(QSP_Packet));
    if(result) // Set was successful, params are loaded from mem, send ack.
        {
      QSP_ClearPayload(QSP_Packet);
      QSP_SetAddress(QSP_Packet, QSP_Status);
      QSP_SetControl(QSP_Packet, QSP_StatusAck);
        }
  }
  if(!result)
  {
    QSP_ClearPayload(QSP_Packet);
    QSP_SetAddress(QSP_Packet, QSP_Status);
    QSP_SetControl(QSP_Packet, QSP_StatusNack);
  }
}

