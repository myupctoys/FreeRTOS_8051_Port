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
 * Create a single persistent task which periodically dynamically creates another 
 * four tasks.  The original task is called the creator task, the four tasks it 
 * creates are called suicidal tasks.
 *
 * Two of the created suicidal tasks kill one other suicidal task before killing 
 * themselves - leaving just the original task remaining.  
 *
 * The creator task must be spawned after all of the other demo application tasks 
 * as it keeps a check on the number of tasks under the scheduler control.  The 
 * number of tasks it expects to see running should never be greater than the 
 * number of tasks that were in existence when the creator task was spawned, plus 
 * one set of four suicidal tasks.  If this number is exceeded an error is flagged.
 *
 * \page DeathC death.c
 * \ingroup DemoFiles
 * <HR>
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
#include "death.h"
#include "print.h"

#define deathSTACK_SIZE		( ( unsigned portSHORT ) 512 )

/* The task originally created which is responsible for periodically dynamically 
creating another four tasks. */
static void vCreateTasks( void *pvParameters );

/* The task function of the dynamically created tasks. */
static void vSuicidalTask( void *pvParameters );

/* A variable which is incremented every time the dynamic tasks are created.  This 
is used to check that the task is still running. */
static volatile portSHORT sCreationCount = 0;

/* Used to store the number of tasks that were originally running so the creator 
task can tell if any of the suicidal tasks have failed to die. */
static volatile portSHORT sTasksRunningAtStart = 0;
static const portSHORT sMaxNumberOfExtraTasksRunning = 4;

/* Used to store a handle to the tasks that should be killed by a suicidal task, 
before it kills itself. */
xTaskHandle xCreatedTask1, xCreatedTask2;

/*-----------------------------------------------------------*/

void vCreateSuicidalTasks( unsigned portCHAR ucPriority )
{
unsigned portCHAR *pucPriority;

	/* Create the Creator tasks - passing in as a parameter the priority at which 
	the suicidal tasks should be created. */
	pucPriority = ( unsigned portCHAR * ) pvPortMalloc( sizeof( unsigned portCHAR ) );
	*pucPriority = ucPriority;

	sTaskCreate( vCreateTasks, "CREATOR", deathSTACK_SIZE, ( void * ) pucPriority, ucPriority, NULL );

	/* Record the number of tasks that are running now so we know if any of the 
	suicidal tasks have failed to be killed. */
	sTasksRunningAtStart = ( portSHORT ) usTaskGetNumberOfTasks();
}
/*-----------------------------------------------------------*/

static void vSuicidalTask( void *pvParameters )
{
portDOUBLE d1, d2;
xTaskHandle xTaskToKill;
const portTickType xDelay = ( portTickType ) 500 / portTICKS_PER_MS;

	if( pvParameters != NULL )
	{
		/* This task is periodically created four times.  Tow created tasks are 
		passed a handle to the other task so it can kill it before killing itself.  
		The other task is passed in null. */
		xTaskToKill = *( xTaskHandle* )pvParameters;
	}
	else
	{
		xTaskToKill = NULL;
	}

	for( ;; )
	{
		/* Do something random just to use some stack and registers. */
		d1 = 2.4;
		d2 = 89.2;
		d2 *= d1;
		vTaskDelay( xDelay );

		if( xTaskToKill != NULL )
		{
			/* Make sure the other task has a go before we delete it. */
			vTaskDelay( ( portTickType ) 0 );
			/* Kill the other task that was created by vCreateTasks(). */
			vTaskDelete( xTaskToKill );
			/* Kill ourselves. */
			vTaskDelete( NULL );
		}
	}
}/*lint !e818 !e550 Function prototype must be as per standard for task functions. */
/*-----------------------------------------------------------*/

static void vCreateTasks( void *pvParameters )
{
const portTickType xDelay = ( portTickType ) 1000 / portTICKS_PER_MS;
unsigned portCHAR ucPriority;
const portCHAR * const pcTaskStartMsg = "Create task started.\r\n";

	/* Queue a message for printing to say the task has started. */
	vPrintDisplayMessage( &pcTaskStartMsg );

	ucPriority = *( unsigned portCHAR * ) pvParameters;
	vPortFree( pvParameters );

	for( ;; )
	{
		/* Just loop round, delaying then creating the four suicidal tasks. */
		vTaskDelay( xDelay );

		sTaskCreate( vSuicidalTask, "SUICIDE1", deathSTACK_SIZE, NULL, ucPriority, &xCreatedTask1 );
		sTaskCreate( vSuicidalTask, "SUICIDE2", deathSTACK_SIZE, &xCreatedTask1, ucPriority, NULL );

		sTaskCreate( vSuicidalTask, "SUICIDE1", deathSTACK_SIZE, NULL, ucPriority, &xCreatedTask2 );
		sTaskCreate( vSuicidalTask, "SUICIDE2", deathSTACK_SIZE, &xCreatedTask2, ucPriority, NULL );

		++sCreationCount;
	}
}
/*-----------------------------------------------------------*/

/* This is called to check that the creator task is still running and that there 
are not any more than four extra tasks. */
portSHORT sIsCreateTaskStillRunning( void )
{
static portSHORT sLastCreationCount = 0;
portSHORT sReturn = pdTRUE, sTasksRunningNow;

	if( sLastCreationCount == sCreationCount )
	{
		sReturn = pdFALSE;
	}
	
	sTasksRunningNow = ( portSHORT ) usTaskGetNumberOfTasks();

	if( sTasksRunningNow < sTasksRunningAtStart )
	{
		sReturn = pdFALSE;
	}
	else if( ( sTasksRunningNow - sTasksRunningAtStart ) > sMaxNumberOfExtraTasksRunning )
	{
		sReturn = pdFALSE;
	}
	else
	{
		/* Everything is okay. */
	}

	return sReturn;
}


