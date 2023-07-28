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


/* BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER FOR DEMO PURPOSES */
#include <stdlib.h>
#include "projdefs.h"
#include "portable.h"
#include "queue.h"
#include "task.h"
#include "serial.h"

/* Constants required to setup the serial control register. */
#define ser8_BIT_MODE			( ( unsigned portCHAR ) 0x40 )
#define serRX_ENABLE			( ( unsigned portCHAR ) 0x10 )

/* Constants to setup the timer used to generate the baud rate. */
#define serCLOCK_DIV_48			( ( unsigned portCHAR ) 0x03 )
#define serUSE_PRESCALED_CLOCK	( ( unsigned portCHAR ) 0x10 )
#define ser8BIT_WITH_RELOAD		( ( unsigned portCHAR ) 0x20 )
#define serSMOD					( ( unsigned portCHAR ) 0x10 )

static xQueueHandle xRxedChars; 
static xQueueHandle xCharsForTx; 

data static unsigned portCHAR ucTxEmpty;

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal( unsigned portLONG ulWantedBaud, unsigned portCHAR ucQueueLength )
{
unsigned portLONG ulReloadValue;
const portFLOAT fBaudConst = ( portFLOAT ) portCPU_CLOCK_HZ * ( portFLOAT ) 2.0;
unsigned portCHAR ucOriginalSFRPage;

	portENTER_CRITICAL();
	{
		ucOriginalSFRPage = SFRPAGE;
		SFRPAGE = 0;

		ucTxEmpty = pdTRUE;

		/* Create the queues used by the com test task. */
		xRxedChars = xQueueCreate( ucQueueLength, ( unsigned portCHAR ) sizeof( portCHAR ) );
		xCharsForTx = xQueueCreate( ucQueueLength, ( unsigned portCHAR ) sizeof( portCHAR ) );
	
		/* Calculate the baud rate to use timer 1. */
		ulReloadValue = ( unsigned portLONG ) ( ( ( portFLOAT ) 256 - ( fBaudConst / ( portFLOAT ) ( 32 * ulWantedBaud ) ) ) + ( portFLOAT ) 0.5 );

		/* Set timer one for desired mode of operation. */
		TMOD &= 0x08;
		TMOD |= ser8BIT_WITH_RELOAD;
		SSTA0 |= serSMOD;

		/* Set the reload and start values for the time. */
		TL1 = ( unsigned portCHAR ) ulReloadValue;
		TH1 = ( unsigned portCHAR ) ulReloadValue;

		/* Setup the control register for standard n, 8, 1 - variable baud rate. */
		SCON = ser8_BIT_MODE | serRX_ENABLE;

		/* Enable the serial port interrupts */
		ES = 1;

		/* Start the timer. */
		TR1 = 1;

		SFRPAGE = ucOriginalSFRPage;
	}
	portEXIT_CRITICAL();
	
	/* Unlike some ports, this serial code does not allow for more than one
	com port.  We therefore don't return a pointer to a port structure and can
	instead just return NULL. */
	return NULL;
}
/*-----------------------------------------------------------*/

void vSerialISR( void ) interrupt 4
{
portCHAR cChar, cTaskWokenByRx = pdFALSE, cTaskWokenByTx = pdFALSE;

	/* 8051 port interrupt routines MUST be placed within a critical section
	if taskYIELD() is used within the ISR! */

	portENTER_CRITICAL();
	{
		if( RI ) 
		{
			/* Get the character and post it on the queue of Rxed characters.
			If the post causes a task to wake force a context switch as the woken task
			may have a higher priority than the task we have interrupted. */
			cChar = SBUF;
			RI = 0;

			if( cQueueSendFromISR( xRxedChars, &cChar, pdFALSE ) )
			{
				cTaskWokenByRx = ( portCHAR ) pdTRUE;
			}
		}

		if( TI ) 
		{
			if( cQueueReceiveFromISR( xCharsForTx, &cChar, &cTaskWokenByTx ) == ( portCHAR ) pdTRUE )
			{
				/* Send the next character queued for Tx. */
				SBUF = cChar;
			}
			else
			{
				/* Queue empty, nothing to send. */
				ucTxEmpty = pdTRUE;
			}

			TI = 0;
		}
	
		if( cTaskWokenByRx || cTaskWokenByTx )
		{
			portYIELD();
		}
	}
	portEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

portCHAR cSerialGetChar( xComPortHandle pxPort, portCHAR *pcRxedChar, portTickType xBlockTime )
{
	/* There is only one port supported. */
	( void ) pxPort;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	if( cQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
	{
		return ( portCHAR ) pdTRUE;
	}
	else
	{
		return ( portCHAR ) pdFALSE;
	}
}
/*-----------------------------------------------------------*/

portCHAR cSerialPutChar( xComPortHandle pxPort, portCHAR cOutChar, portTickType xBlockTime )
{
portCHAR cReturn;

	/* There is only one port supported. */
	( void ) pxPort;

	portENTER_CRITICAL();
	{
		if( ucTxEmpty == pdTRUE )
		{
			SBUF = cOutChar;
			ucTxEmpty = pdFALSE;
			cReturn = ( portCHAR ) pdTRUE;
		}
		else
		{
			cReturn = cQueueSend( xCharsForTx, &cOutChar, xBlockTime );

			if( cReturn == pdFALSE )
			{
				cReturn = pdTRUE;
			}
		}
	}
	portEXIT_CRITICAL();

	return cReturn;
}
/*-----------------------------------------------------------*/

void vSerialClose( xComPortHandle xPort )
{
	/* Not implemented in this port. */
	( void ) xPort;
}
/*-----------------------------------------------------------*/





