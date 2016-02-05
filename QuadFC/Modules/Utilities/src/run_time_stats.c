/*
 * run_time_stats.c
 *
 * Copyright (C) 2016 Martin Lundh
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

#include "Utilities/inc/run_time_stats.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
void QuadFC_RuntimeStats( uint8_t * buffer , uint16_t bufferLength)
{
#ifdef QuadFCStats
//#if 1
  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  uint32_t ulTotalTime, ulStatsAsPercentage;

  *buffer = 0x00;

  /* Take a snapshot of the number of tasks in case it changes while this
    function is executing. */
  uxArraySize = uxTaskGetNumberOfTasks();

  /* Allocate an array index for each task. */
  pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

  if( pxTaskStatusArray != NULL )
  {
    /* Generate the (binary) data. */
    uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalTime );

    /* For percentage calculations. */
    ulTotalTime /= 100UL;


    /* Avoid divide by zero errors. */
    if( ulTotalTime > 0 )
    {
      /* Create a human readable table from the binary data. */
      for( x = 0; x < uxArraySize; x++ )
      {
        /* What percentage of the total run time has the task used?
        This will always be rounded down to the nearest integer.
        ulTotalRunTimeDiv100 has already been divided by 100. */
        ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalTime;

        if( ulStatsAsPercentage > 0UL )
        {
          snprintf((char *) buffer, bufferLength , "\n%s,%u,%u,%u,%u%%,",
              pxTaskStatusArray[ x ].pcTaskName,
              ( unsigned int ) pxTaskStatusArray[ x ].uxCurrentPriority,
              ( unsigned int ) pxTaskStatusArray[ x ].usStackHighWaterMark,
              ( unsigned int ) pxTaskStatusArray[ x ].ulRunTimeCounter,
              ( unsigned int ) ulStatsAsPercentage );
        }
        else
        {
          snprintf((char *) buffer, bufferLength , "\n%s,%u,%u,%u,<1%%,",
              pxTaskStatusArray[ x ].pcTaskName,
              ( unsigned int ) pxTaskStatusArray[ x ].uxCurrentPriority,
              ( unsigned int ) pxTaskStatusArray[ x ].usStackHighWaterMark,
              ( unsigned int ) pxTaskStatusArray[ x ].ulRunTimeCounter );
        }

        size_t len = strlen((char *) buffer );
        buffer += len;
        bufferLength -= len;
      }
    }

    /* Free the array again. */
    vPortFree( pxTaskStatusArray );
  }
#else
  snprintf((char *) buffer, bufferLength , "Run time statistics is not available");
#endif
}

