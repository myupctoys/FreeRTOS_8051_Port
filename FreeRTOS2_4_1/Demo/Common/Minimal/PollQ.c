/*
	FreeRTOS V2.4.1 - Copyright (C) 2003, 2004 Richard Barry.

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

	***************************************************************************
	See http://www.FreeRTOS.org for documentation, latest information, license 
	and contact details.  Please ensure to read the configuration and relevant 
	port sections of the online documentation.
	***************************************************************************
*/

/*
 * This version of PollQ. c is for use on systems that have limited stack
 * space and no display facilities.  The complete version can be found in
 * the Demo/Common/Full directory.
 *
 * Creates two tasks that communicate over a single queue.  One task acts as a 
 * producer, the other a consumer.  
 *
 * The producer loops for three iteration, posting an incrementing number onto the 
 * queue each cycle.  It then delays for a fixed period before doing exactly the 
 * same again.
 *
 * The consumer loops emptying the queue.  Each item removed from the queue is 
 * checked to ensure it contains the expected value.  When the queue is empty it 
 * blocks for a fixed period, then does the same again.
 *
 * All queue access is performed without blocking.  The consumer completely empties 
 * the queue each time it runs so the producer should never find the queue full.  
 *
 * An error is flagged if the consumer obtains an unexpected value or the producer 
 * find the queue is full.
 */

/*
Changes from V2.0.0

	+ Delay periods are now specified using variables and constants of
	  portTickType rather than unsigned portLONG.
*/

#include <stdlib.h>

/* Scheduler include files. */
#include "projdefs.h"
#include "portable.h"
#include "task.h"
#include "queue.h"

/* Demo program include files. */
#include "PollQ.h"

#define pollqSTACK_SIZE			portMINIMAL_STACK_SIZE
#define pollqQUEUE_SIZE			( ( unsigned portCHAR ) 10 )
#define pollqDELAY				( ( portTickType ) 200 / portTICKS_PER_MS )
#define pollqNO_DELAY			( ( portTickType ) 0 )
#define pollqVALUES_TO_PRODUCE	( ( signed portCHAR ) 3 )
#define pollqINITIAL_VALUE		( ( signed portCHAR ) 0 )

/* The task that posts the incrementing number onto the queue. */
static void vPolledQueueProducer( void *pvParameters );

/* The task that empties the queue. */
static void vPolledQueueConsumer( void *pvParameters );

/* Variables that are used to check that the tasks are still running with no 
errors. */
static volatile signed portCHAR cPollingConsumerCount = pollqINITIAL_VALUE, cPollingProducerCount = pollqINITIAL_VALUE;

/*-----------------------------------------------------------*/

void vStartPolledQueueTasks( unsigned portCHAR ucPriority )
{
static xQueueHandle xPolledQueue;

	/* Create the queue used by the producer and consumer. */
	xPolledQueue = xQueueCreate( pollqQUEUE_SIZE, ( unsigned portCHAR ) sizeof( unsigned portSHORT ) );

	/* Spawn the producer and consumer. */
	sTaskCreate( vPolledQueueConsumer, ( const signed portCHAR * const ) "QConsNB", pollqSTACK_SIZE, ( void * ) &xPolledQueue, ucPriority, ( xTaskHandle * ) NULL );
	sTaskCreate( vPolledQueueProducer, ( const signed portCHAR * const ) "QProdNB", pollqSTACK_SIZE, ( void * ) &xPolledQueue, ucPriority, ( xTaskHandle * ) NULL );
}
/*-----------------------------------------------------------*/

static void vPolledQueueProducer( void *pvParameters )
{
unsigned portSHORT usValue = ( unsigned portSHORT ) 0;
signed portCHAR cError = ( signed portCHAR ) pdFALSE, cLoop;

	for( ;; )
	{		
		for( cLoop = 0; cLoop < pollqVALUES_TO_PRODUCE; cLoop++ )
		{
			/* Send an incrementing number on the queue without blocking. */
			if( cQueueSend( *( ( xQueueHandle * ) pvParameters ), ( void * ) &usValue, pollqNO_DELAY ) != ( signed portCHAR ) pdPASS )
			{
				/* We should never find the queue full so if we get here there
				has been an error. */
				cError = ( signed portCHAR ) pdTRUE;
			}
			else
			{
				if( cError == ( signed portCHAR ) pdFALSE )
				{
					/* If an error has ever been recorded we stop incrementing the 
					check variable. */
					portENTER_CRITICAL();
						cPollingProducerCount++;
					portEXIT_CRITICAL();
				}

				/* Update the value we are going to post next time around. */
				usValue++;
			}
		}

		/* Wait before we start posting again to ensure the consumer runs and 
		empties the queue. */
		vTaskDelay( pollqDELAY );
	}
}  /*lint !e818 Function prototype must conform to API. */
/*-----------------------------------------------------------*/

static void vPolledQueueConsumer( void *pvParameters )
{
unsigned portSHORT usData, usExpectedValue = ( unsigned portSHORT ) 0;
signed portCHAR cError = ( signed portCHAR ) pdFALSE;

	for( ;; )
	{		
		/* Loop until the queue is empty. */
		while( ucQueueMessagesWaiting( *( ( xQueueHandle * ) pvParameters ) ) )
		{
			if( cQueueReceive( *( ( xQueueHandle * ) pvParameters ), &usData, pollqNO_DELAY ) == ( signed portCHAR ) pdPASS )
			{
				if( usData != usExpectedValue )
				{
					/* This is not what we expected to receive so an error has 
					occurred. */
					cError = ( signed portCHAR ) pdTRUE;

					/* Catch-up to the value we received so our next expected 
					value should again be correct. */
					usExpectedValue = usData;
				}
				else
				{
					if( cError == ( signed portCHAR ) pdFALSE )
					{
						/* Only increment the check variable if no errors have 
						occurred. */
						portENTER_CRITICAL();
							cPollingConsumerCount++;
						portEXIT_CRITICAL();
					}
				}

				/* Next time round we would expect the number to be one higher. */
				usExpectedValue++;
			}
		}

		/* Now the queue is empty we block, allowing the producer to place more 
		items in the queue. */
		vTaskDelay( pollqDELAY );
	}
} /*lint !e818 Function prototype must conform to API. */
/*-----------------------------------------------------------*/

/* This is called to check that all the created tasks are still running with no errors. */
portSHORT sArePollingQueuesStillRunning( void )
{
portSHORT sReturn;

	/* Check both the consumer and producer poll count to check they have both
	been changed since out last trip round.  We do not need a critical section
	around the check variables as this is called from a higher priority than 
	the other tasks that access the same variables. */
	if( ( cPollingConsumerCount == pollqINITIAL_VALUE ) ||
		( cPollingProducerCount == pollqINITIAL_VALUE ) 
	  )
	{
		sReturn = pdFALSE;
	}
	else
	{
		sReturn = pdTRUE;
	}

	/* Set the check variables back down so we know if they have been
	incremented the next time around. */
	cPollingConsumerCount = pollqINITIAL_VALUE;
	cPollingProducerCount = pollqINITIAL_VALUE;

	return sReturn;
}
