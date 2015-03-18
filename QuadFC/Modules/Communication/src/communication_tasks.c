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

static void Com_TxTask( void *pvParameters );
static void Com_RxTask( void *pvParameters );


/**
 * Initialize the communication module.
 * @return
 */
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

    if ( nr_bytes_received >= 1 )
    {
      for ( int i = 0; i < nr_bytes_received; i++, k++ )
      {
        rxSLIP->payload[k] = receive_buffer[i];
        if ( rxSLIP->payload[k] == frame_boundary_octet )
        {
          if ( k > 4 ) // Last boundary of frame. Frame minimum length = 4
          {
            rxSLIP->packetSize = k;
            k = 0;
            /*handle communication*/
            Slip_DePacketize(QSP_GetPacketPtr(rxPacket), QSP_GetAvailibleSize(rxPacket), rxSLIP);

            uint8_t result = Com_CheckCRC(rxPacket);
            if(result)
            {
              result = Com_HandleQSP(rxPacket, rxRespPacket);
              if(result) // ok to send resp.
              {
                Com_SendQSP(rxRespPacket);
              }
            }
            else // CRC errpr.
            {
              QSP_ClearPayload(rxRespPacket);
              QSP_SetAddress(rxRespPacket, QSP_Status);
              QSP_SetControl(rxRespPacket, QSP_StatusCrcError);
              Com_SendQSP(rxRespPacket);
            }

          }
          else // First boundary of frame.
          {
            k = 0;
            rxSLIP->payload[k] = receive_buffer[i];

          }
        }
      }
    }
  }
}


uint8_t Com_HandleQSP( QSP_t *QSP_packet, QSP_t *QSP_RspPacket)
{
  uint8_t result = 0;
  switch ( QSP_GetAddress(QSP_packet) )
  {
  case QSP_Parameters:
    result = Com_HandleParameters(QSP_packet, QSP_RspPacket);
    break;
  case QSP_Status:
    result = Com_HandleStatus(QSP_packet, QSP_RspPacket);
    break;
  default:
    break;
  }

  return result;
}

uint8_t Com_HandleStatus(QSP_t *QSP_packet, QSP_t *QSP_RspPacket)
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
uint8_t Com_HandleParameters(QSP_t *QSP_packet, QSP_t *QSP_RspPacket)
{
  QSP_ClearPayload(QSP_RspPacket);
  uint8_t result = 0;
  switch ( QSP_GetControl(QSP_packet) )
  {
  case QSP_ParamGetTree:
    result = Param_DumpFromRoot(Param_GetRoot(), QSP_GetPayloadPtr(QSP_RspPacket), QSP_GetAvailibleSize(QSP_RspPacket) - QSP_GetHeaderdSize());
    if(result)
    {
      QSP_SetPayloadSize(QSP_RspPacket, strlen((char *)QSP_GetPayloadPtr(QSP_RspPacket)));
      QSP_SetAddress(QSP_RspPacket, QSP_Parameters);
      QSP_SetControl(QSP_RspPacket, QSP_ParamSetTree);
    }
    else
    {
      QSP_ClearPayload(QSP_RspPacket);
      QSP_SetAddress(QSP_RspPacket, QSP_Status);
      QSP_SetControl(QSP_RspPacket, QSP_StatusNack);
    }
    break;
  case QSP_ParamSetTree:
    result = Param_SetFromRoot(Param_GetRoot(), QSP_GetPayloadPtr(QSP_packet), QSP_GetPayloadSize(QSP_packet));
    QSP_SetAddress(QSP_RspPacket, QSP_Status);
    if(result)
    {
      QSP_SetControl(QSP_RspPacket, QSP_StatusAck);
    }
    else
    {
      QSP_SetControl(QSP_RspPacket, QSP_StatusNack);
    }
    break;
  default:
    break;
  }

  return result;
}


uint8_t Com_SendQSP( QSP_t *packet )
{
  //TODO add check to make sure sending was successful.
  QSP_GiveOwnership(packet);
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
    return 1;
  }
  return 0;
}

uint8_t Com_CheckCRC( QSP_t *packet) // TODO
{
  uint16_t payloadSize = QSP_GetPayloadSize(packet);

  //Calculate and compare CRC16 checksum.
  uint16_t crcCalc;
  uint16_t crcMsg = 0;
  crcCalc = crcFast( QSP_GetPacketPtr(packet), (payloadSize + QSP_GetHeaderdSize()));

  crcMsg |= (uint16_t) ((QSP_GetPayloadPtr(packet)[payloadSize + QSP_GetHeaderdSize()] << 8) & 0Xff00);
  crcMsg |= (uint16_t) ((QSP_GetPayloadPtr(packet)[payloadSize + QSP_GetHeaderdSize() + 1]) & 0Xff);
  if ( crcCalc != crcMsg )
  {
    return 1;
  }
  return 0;
}



