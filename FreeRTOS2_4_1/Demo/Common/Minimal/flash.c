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

/**
 * This version of flash .c is for use on systems that have limited stack space
 * and no display facilities.  The complete version can be found in the 
 * Demo/Common/Full directory.
 * 
 * Three tasks are created, each of which flash an LED at a different rate.  The first 
 * LED flashes every 200ms, the second every 400ms, the third every 600ms.
 *
 * The LED flash tasks provide instant visual feedback.  They show that the scheduler 
 * is still operational.
 *
 * The PC port uses the standard parallel port for outputs, the Flashlite 186 port 
 * uses IO port F and the AVR port port B.
 *
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

/* Demo program include files. */
#include "partest.h"
#include "flash.h"

#define ledSTACK_SIZE		portMINIMAL_STACK_SIZE
#define ledNUMBER_OF_LEDS	( ( unsigned portCHAR ) 3 )
#define ledFLASH_RATE_BASE	( ( portTickType ) 333 )

/* Variable used by the created tasks to calculate the LED number to use, and
the rate at which they should flash the LED. */
static volatile unsigned portCHAR ucFlashTaskNumber = ( unsigned portCHAR ) 0;

/* The task that is created three times. */
static void vLEDFlashTask( void *pvParameters );

/*-----------------------------------------------------------*/

void vStartLEDFlashTasks( unsigned portCHAR ucPriority )
{
signed portCHAR cLEDTask;

	/* Create the three tasks. */
	for( cLEDTask = 0; cLEDTask < ledNUMBER_OF_LEDS; ++cLEDTask )
	{
		/* Spawn the task. */
		sTaskCreate( vLEDFlashTask, ( const signed portCHAR * const ) "LEDx", ledSTACK_SIZE, NULL, ucPriority, ( xTaskHandle * ) NULL );
	}
}
/*-----------------------------------------------------------*/

static void vLEDFlashTask( void *pvParameters )
{
portTickType xFlashRate;
unsigned portCHAR ucLED;

	/* Calculate the LED and flash rate. */
	portENTER_CRITICAL();
	{
		/* See which of the eight LED's we should use. */
		ucLED = ucFlashTaskNumber;

		/* Update so the next task uses the next LED. */
		ucFlashTaskNumber++;
	}
	portEXIT_CRITICAL();

	xFlashRate = ledFLASH_RATE_BASE + ( ledFLASH_RATE_BASE * ( portTickType ) ucLED );
	xFlashRate /= portTICKS_PER_MS;

	/* We will turn the LED on and off again in the delay period, so each
	delay is only half the total period. */
	xFlashRate /= ( portTickType ) 2;

	for(;;)
	{
		/* Delay for half the flash period then turn the LED on. */
		vTaskDelay( xFlashRate );
		vParTestToggleLED( ucLED );

		/* Delay for half the flash period then turn the LED off. */
		vTaskDelay( xFlashRate );
		vParTestToggleLED( ucLED );
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

