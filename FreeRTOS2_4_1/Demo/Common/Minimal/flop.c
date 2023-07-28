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
 * Creates eight tasks, each of which loops continuously performing an (emulated) 
 * floating point calculation.
 *
 * All the tasks run at the idle priority and never block or yield.  This causes 
 * all eight tasks to time slice with the idle task.  Running at the idle priority 
 * means that these tasks will get pre-empted any time another task is ready to run
 * or a time slice occurs.  More often than not the pre-emption will occur mid 
 * calculation, creating a good test of the schedulers context switch mechanism - a 
 * calculation producing an unexpected result could be a symptom of a corruption in 
 * the context of a task.
 */

#include <stdlib.h>
#include <math.h>

/* Scheduler include files. */
#include "projdefs.h"
#include "portable.h"
#include "task.h"

/* Demo program include files. */
#include "flop.h"

#define mathSTACK_SIZE		portMINIMAL_STACK_SIZE
#define mathNUMBER_OF_TASKS  ( 8 )

/* Four tasks, each of which performs a different floating point calculation.  
Each of the four is created twice. */
static void vCompetingMathTask1( void *pvParameters );
static void vCompetingMathTask2( void *pvParameters );
static void vCompetingMathTask3( void *pvParameters );
static void vCompetingMathTask4( void *pvParameters );

/* These variables are used to check that all the tasks are still running.  If a 
task gets a calculation wrong it will
stop incrementing its check variable. */
static volatile unsigned portSHORT usTaskCheck[ mathNUMBER_OF_TASKS ] = { ( unsigned portSHORT ) 0 };

/*-----------------------------------------------------------*/

void vStartMathTasks( unsigned portCHAR ucPriority )
{
	sTaskCreate( vCompetingMathTask1, "Math1", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 0 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask2, "Math2", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 1 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask3, "Math3", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 2 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask4, "Math4", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 3 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask1, "Math5", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 4 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask2, "Math6", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 5 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask3, "Math7", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 6 ] ), ucPriority, NULL );
	sTaskCreate( vCompetingMathTask4, "Math8", mathSTACK_SIZE, ( void * ) &( usTaskCheck[ 7 ] ), ucPriority, NULL );
}
/*-----------------------------------------------------------*/

static void vCompetingMathTask1( void *pvParameters )
{
volatile portDOUBLE d1, d2, d3, d4;
volatile unsigned portSHORT *pusTaskCheckVariable;
const portDOUBLE dAnswer = ( 123.4567 + 2345.6789 ) * -918.222;
portSHORT sError = pdFALSE;

	/* The variable this task increments to show it is still running is passed in 
	as the parameter. */
	pusTaskCheckVariable = ( unsigned portSHORT * ) pvParameters;

	/* Keep performing a calculation and checking the result against a constant. */
	for(;;)
	{
		d1 = 123.4567;
		d2 = 2345.6789;
		d3 = -918.222;

		d4 = ( d1 + d2 ) * d3;

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif

		/* If the calculation does not match the expected constant, stop the 
		increment of the check variable. */
		if( fabs( d4 - dAnswer ) > 0.001 )
		{
			sError = pdTRUE;
		}

		if( sError == pdFALSE )
		{
			/* If the calculation has always been correct, increment the check 
			variable so we know this task is still running okay. */
			( *pusTaskCheckVariable )++;
		}

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif

	}
}
/*-----------------------------------------------------------*/

static void vCompetingMathTask2( void *pvParameters )
{
volatile portDOUBLE d1, d2, d3, d4;
volatile unsigned portSHORT *pusTaskCheckVariable;
const portDOUBLE dAnswer = ( -389.38 / 32498.2 ) * -2.0001;
portSHORT sError = pdFALSE;

	/* The variable this task increments to show it is still running is passed in 
	as the parameter. */
	pusTaskCheckVariable = ( unsigned portSHORT * ) pvParameters;

	/* Keep performing a calculation and checking the result against a constant. */
	for( ;; )
	{
		d1 = -389.38;
		d2 = 32498.2;
		d3 = -2.0001;

		d4 = ( d1 / d2 ) * d3;

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif
		
		/* If the calculation does not match the expected constant, stop the 
		increment of the check variable. */
		if( fabs( d4 - dAnswer ) > 0.001 )
		{
			sError = pdTRUE;
		}

		if( sError == pdFALSE )
		{
			/* If the calculation has always been correct, increment the check 
			variable so we know
			this task is still running okay. */
			( *pusTaskCheckVariable )++;
		}

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif
	}
}
/*-----------------------------------------------------------*/

static void vCompetingMathTask3( void *pvParameters )
{
volatile portDOUBLE *pdArray, dTotal1, dTotal2, dDifference;
volatile unsigned portSHORT *pusTaskCheckVariable;
const unsigned portSHORT usArraySize = 10;
unsigned portSHORT usPosition;
portSHORT sError = pdFALSE;

	/* The variable this task increments to show it is still running is passed in 
	as the parameter. */
	pusTaskCheckVariable = ( unsigned portSHORT * ) pvParameters;

	pdArray = ( portDOUBLE * ) pvPortMalloc( usArraySize * sizeof( portDOUBLE ) );

	/* Keep filling an array, keeping a running total of the values placed in the 
	array.  Then run through the array adding up all the values.  If the two totals 
	do not match, stop the check variable from incrementing. */
	for( ;; )
	{
		dTotal1 = 0.0;
		dTotal2 = 0.0;

		for( usPosition = 0; usPosition < usArraySize; usPosition++ )
		{
			pdArray[ usPosition ] = ( portDOUBLE ) usPosition + 5.5;
			dTotal1 += ( portDOUBLE ) usPosition + 5.5;	
		}

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif

		for( usPosition = 0; usPosition < usArraySize; usPosition++ )
		{
			dTotal2 += pdArray[ usPosition ];
		}

		dDifference = dTotal1 - dTotal2;
		if( fabs( dDifference ) > 0.001 )
		{
			sError = pdTRUE;
		}

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif

		if( sError == pdFALSE )
		{
			/* If the calculation has always been correct, increment the check 
			variable so we know	this task is still running okay. */
			( *pusTaskCheckVariable )++;
		}
	}
}
/*-----------------------------------------------------------*/

static void vCompetingMathTask4( void *pvParameters )
{
volatile portDOUBLE *pdArray, dTotal1, dTotal2, dDifference;
volatile unsigned portSHORT *pusTaskCheckVariable;
const unsigned portSHORT usArraySize = 10;
unsigned portSHORT usPosition;
portSHORT sError = pdFALSE;

	/* The variable this task increments to show it is still running is passed in 
	as the parameter. */
	pusTaskCheckVariable = ( unsigned portSHORT * ) pvParameters;

	pdArray = ( portDOUBLE * ) pvPortMalloc( usArraySize * sizeof( portDOUBLE ) );

	/* Keep filling an array, keeping a running total of the values placed in the 
	array.  Then run through the array adding up all the values.  If the two totals 
	do not match, stop the check variable from incrementing. */
	for( ;; )
	{
		dTotal1 = 0.0;
		dTotal2 = 0.0;

		for( usPosition = 0; usPosition < usArraySize; usPosition++ )
		{
			pdArray[ usPosition ] = ( portDOUBLE ) usPosition * 12.123;
			dTotal1 += ( portDOUBLE ) usPosition * 12.123;	
		}

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif

		for( usPosition = 0; usPosition < usArraySize; usPosition++ )
		{
			dTotal2 += pdArray[ usPosition ];
		}

		dDifference = dTotal1 - dTotal2;
		if( fabs( dDifference ) > 0.001 )
		{
			sError = pdTRUE;
		}

		#if portUSE_PREEMPTION == 0
			taskYIELD();
		#endif

		if( sError == pdFALSE )
		{
			/* If the calculation has always been correct, increment the check 
			variable so we know	this task is still running okay. */
			( *pusTaskCheckVariable )++;
		}
	}
}
/*-----------------------------------------------------------*/

/* This is called to check that all the created tasks are still running. */
portSHORT sAreMathsTaskStillRunning( void )
{
/* Keep a history of the check variables so we know if they have been incremented 
since the last call. */
static unsigned portSHORT usLastTaskCheck[ mathNUMBER_OF_TASKS ] = { ( unsigned portSHORT ) 0 };
portSHORT sReturn = pdTRUE, sTask;

	/* Check the maths tasks are still running by ensuring their check variables 
	are still incrementing. */
	for( sTask = 0; sTask < mathNUMBER_OF_TASKS; sTask++ )
	{
		if( usTaskCheck[ sTask ] == usLastTaskCheck[ sTask ] )
		{
			/* The check has not incremented so an error exists. */
			sReturn = pdFALSE;
		}

		usLastTaskCheck[ sTask ] = usTaskCheck[ sTask ];
	}

	return sReturn;
}


