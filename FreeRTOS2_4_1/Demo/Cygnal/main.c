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
 * Creates the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main. c also creates four other tasks:
 * 
 * 1) vErrorChecks()
 * This only executes every few seconds but has the highest priority so is 
 * guaranteed to get processor time.  Its main function is to check that all 
 * the standard demo application tasks are still operational and have not
 * experienced any errors.  vErrorChecks() will toggle the on board LED
 * every mainNO_ERROR_FLASH_PERIOD milliseconds if none of the demo application
 * tasks have reported an error.  Should any task report an error at any time
 * the rate at which the on board LED is toggled is increased to 
 * mainERROR_FLASH_PERIOD - providing visual feedback that something has gone
 * wrong.
 *
 * 2) vRegisterCheck()
 * This is a very simple task that checks that all the registers are always
 * in their expected state.  The task only makes use of the A register, so
 * all the other registers should always contain their initial values.
 * An incorrect value indicates an error in the context switch mechanism.
 * The task operates at the idle priority so will be preempted regularly.
 * Any error will cause the toggle rate of the on board LED to increase to
 * mainERROR_FLASH_PERIOD milliseconds.
 *
 * 3 and 4) vFLOPCheck1() and vFLOPCheck2()
 * These are very basic versions of the standard FLOP tasks.  They are good
 * at detecting errors in the context switch mechanism, and also check that
 * the floating point libraries are correctly built to be re-enterant.  The
 * stack restrictions of the 8051 prevent the use of the standard FLOP demo
 * tasks.
 */

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "projdefs.h"
#include "portable.h"
#include "task.h"

/* Demo application includes. */
#include "partest.h"
#include "flash.h"
#include "integer.h"
#include "PollQ.h"
#include "comtest2.h"
#include "semtest.h"

/* Demo task priorities. */
#define mainLED_TASK_PRIORITY		( tskIDLE_PRIORITY + ( unsigned portCHAR ) 1 )
#define mainQUEUE_POLL_PRIORITY		( tskIDLE_PRIORITY + ( unsigned portCHAR ) 2 )
#define mainCOM_TEST_PRIORITY		( tskIDLE_PRIORITY + ( unsigned portCHAR ) 2 )
#define mainCHECK_TASK_PRIORITY		( tskIDLE_PRIORITY + ( unsigned portCHAR ) 3 )
#define mainSEM_TEST_PRIORITY		( tskIDLE_PRIORITY + ( unsigned portCHAR ) 2 )
#define mainINTEGER_PRIORITY		tskIDLE_PRIORITY

/* Constants required to disable the watchdog. */
#define mainDISABLE_BYTE_1			( ( unsigned portCHAR ) 0xde )
#define mainDISABLE_BYTE_2			( ( unsigned portCHAR ) 0xad )

/* Constants to setup and use the on board LED. */
#define ucLED_BIT					( ( unsigned portCHAR ) 0x40 )
#define mainPORT_1_BIT_6			( ( unsigned portCHAR ) 0x40 )
#define mainENABLE_CROSS_BAR		( ( unsigned portCHAR ) 0x40 )

/* Constants to set the clock frequency. */
#define mainSELECT_INTERNAL_OSC		( ( unsigned portCHAR ) 0x80 )
#define mainDIVIDE_CLOCK_BY_1		( ( unsigned portCHAR ) 0x03 )
#define mainPLL_USES_INTERNAL_OSC	( ( unsigned portCHAR ) 0x04 )
#define mainFLASH_READ_TIMING		( ( unsigned portCHAR ) 0x30 )
#define mainPLL_POWER_ON			( ( unsigned portCHAR ) 0x01 )
#define mainPLL_NO_PREDIVIDE		( ( unsigned portCHAR ) 0x01 )
#define mainPLL_FILTER				( ( unsigned portCHAR ) 0x01 )
#define mainPLL_MULTIPLICATION		( ( unsigned portCHAR ) 0x04 )
#define mainENABLE_PLL				( ( unsigned portCHAR ) 0x02 )
#define mainPLL_LOCKED				( ( unsigned portCHAR ) 0x10 )
#define mainSELECT_PLL_AS_SOURCE	( ( unsigned portCHAR ) 0x02 )

/* Toggle rate for the on board LED - which is dependent on whether or not
an error has been detected. */
#define mainNO_ERROR_FLASH_PERIOD	( ( portTickType ) 5000 )
#define mainERROR_FLASH_PERIOD		( ( portTickType ) 250 )

/* Baud rate used by the serial port tasks. */
#define mainCOM_TEST_BAUD_RATE		( ( unsigned portLONG ) 115200 )

/* Pass an invalid LED number to the COM test task as we don't want it to flash
an LED.  There are only 8 LEDs (excluding the on board LED) wired in and these
are all used by the flash tasks. */
#define mainCOM_TEST_LED			( ( unsigned portCHAR ) 200 )

/* We want the Cygnal to act as much as possible as a standard 8052. */
#define mainAUTO_SFR_OFF			( ( unsigned portCHAR ) 0 )

/* Constants required to setup the IO pins for serial comms. */
#define mainENABLE_COMS 			( ( unsigned portCHAR ) 0x04 )
#define mainCOMS_LINES_TO_PUSH_PULL ( ( unsigned portCHAR ) 0x03 )

/* Pointer passed as a parameter to vRegisterCheck() just so it has some know
values to check for in the DPH, DPL and B registers. */
#define mainDUMMY_POINTER		( ( xdata void * ) 0xabcd )

/* Macro that lets vErrorChecks() know that one of the tasks defined in
main. c has detected an error.  A critical region is used around sLatchError
as it is accessed from vErrorChecks(), which has a higher priority. */ 
#define mainLATCH_ERROR()			\
{									\
	portENTER_CRITICAL();			\
		sLatchedError = pdTRUE;		\
	portEXIT_CRITICAL();			\
}

/*
 * Setup the Cygnal microcontroller for its fastest operation. 
 */
static void prvSetupSystemClock( void );

/*
 * Setup the peripherals, including the on board LED. 
 */
static void prvSetupHardware( void );

/*
 * Toggle the state of the on board LED. 
 */
static void prvToggleOnBoardLED( void );

/*
 * See comments at the top of the file for details. 
 */
static void vErrorChecks( void *pvParameters );

/*
 * See comments at the top of the file for details. 
 */
static void vRegisterCheck( void *pvParameters );

/*
 * See comments at the top of the file for details. 
 */
static void vFLOPCheck1( void *pvParameters );

/*
 * See comments at the top of the file for details. 
 */
static void vFLOPCheck2( void *pvParameters );

/* File scope variable used to communicate the occurrence of an error between
tasks. */
static sLatchedError = pdFALSE;

/*-----------------------------------------------------------*/

/*
 * Starts all the other tasks, then starts the scheduler. 
 */
void main( void )
{
	/* Initialise the hardware including the system clock and on board
	LED. */
	prvSetupHardware();

	/* Initialise the port that controls the external LED's utilized by the
	flash tasks. */
	vParTestInitialise();

	/* Start the used standard demo tasks. */
	vStartLEDFlashTasks( mainLED_TASK_PRIORITY );
	vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
	vStartIntegerMathTasks( mainINTEGER_PRIORITY );
	vAltStartComTestTasks( mainCOM_TEST_PRIORITY, mainCOM_TEST_BAUD_RATE, mainCOM_TEST_LED );
	vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );

	/* Start the tasks defined in this file.  The first three never block so
	must not be used with the co-operative scheduler. */
	#if portUSE_PREEMPTION == 1
	{
		sTaskCreate( vRegisterCheck, "RegChck", portMINIMAL_STACK_SIZE, mainDUMMY_POINTER, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
		sTaskCreate( vFLOPCheck1, "FLOP", portMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
		sTaskCreate( vFLOPCheck2, "FLOP", portMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, ( xTaskHandle * ) NULL );
	}
	#endif 

	sTaskCreate( vErrorChecks, "Check", portMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, ( xTaskHandle * ) NULL );

	/* Finally kick off the scheduler.  This function should never return. */
	vTaskStartScheduler( portUSE_PREEMPTION );

	/* Should never reach here as the tasks will now be executing under control
	of the scheduler. */
}
/*-----------------------------------------------------------*/

/*
 * Setup the hardware prior to using the scheduler.  Most of the Cygnal
 * specific initialisation is performed here leaving standard 8052 setup
 * only in the driver code.
 */
static void prvSetupHardware( void )
{
unsigned portCHAR ucOriginalSFRPage;

	/* Remember the SFR page before it is changed so it can get set back
	before the function exits. */
	ucOriginalSFRPage = SFRPAGE;

	/* Setup the SFR page to access the config SFR's. */
	SFRPAGE = CONFIG_PAGE;

	/* Don't allow the microcontroller to automatically switch SFR page, as the
	SFR page is not stored as part of the task context. */
	SFRPGCN = mainAUTO_SFR_OFF;

	/* Disable the watchdog. */
	WDTCN = mainDISABLE_BYTE_1;
	WDTCN = mainDISABLE_BYTE_2;

	/* Set the on board LED to push pull. */
	P1MDOUT |= mainPORT_1_BIT_6;

	/* Setup the cross bar to enable serial comms here as it is not part of the 
	standard 8051 setup and therefore is not in the driver code. */
	XBR0 |= mainENABLE_COMS;
	P0MDOUT |= mainCOMS_LINES_TO_PUSH_PULL;

	/* Enable the cross bar so our hardware setup takes effect. */
	XBR2 = mainENABLE_CROSS_BAR;

	/* Setup a fast system clock. */
	prvSetupSystemClock();

	/* Return the SFR page. */
	SFRPAGE = ucOriginalSFRPage;
}
/*-----------------------------------------------------------*/

static void prvSetupSystemClock( void )
{
volatile unsigned portSHORT usWait;
const unsigned portSHORT usWaitTime = ( unsigned portSHORT ) 0x2ff;
unsigned portCHAR ucOriginalSFRPage;

	/* Remember the SFR page so we can set it back at the end. */
	ucOriginalSFRPage = SFRPAGE;
	SFRPAGE = CONFIG_PAGE;

	/* Use the internal oscillator set to its fasted frequency. */
	OSCICN = mainSELECT_INTERNAL_OSC | mainDIVIDE_CLOCK_BY_1;

	/* Ensure the clock is stable. */
	for( usWait = 0; usWait < usWaitTime; usWait++ );

	/* Setup the clock source for the PLL. */
	PLL0CN &= ~mainPLL_USES_INTERNAL_OSC;

	/* Change the read timing for the flash ready for the fast clock. */
	SFRPAGE = LEGACY_PAGE;
	FLSCL |= mainFLASH_READ_TIMING;

	/* Turn on the PLL power. */
	SFRPAGE = CONFIG_PAGE;
	PLL0CN |= mainPLL_POWER_ON;

	/* Don't predivide the clock. */
	PLL0DIV = mainPLL_NO_PREDIVIDE;

	/* Set filter for fastest clock. */
	PLL0FLT = mainPLL_FILTER;
	PLL0MUL = mainPLL_MULTIPLICATION;

	/* Ensure the clock is stable. */
	for( usWait = 0; usWait < usWaitTime; usWait++ );

	/* Enable the PLL and wait for it to lock. */
	PLL0CN |= mainENABLE_PLL;
	for( usWait = 0; usWait < usWaitTime; usWait++ )
	{
		if( PLL0CN & mainPLL_LOCKED )
		{
			break;
		}
	}

	/* Select the PLL as the clock source. */
	CLKSEL |= mainSELECT_PLL_AS_SOURCE;

	/* Return the SFR back to its original value. */
	SFRPAGE = ucOriginalSFRPage;
}
/*-----------------------------------------------------------*/

static void prvToggleOnBoardLED( void )
{
	/* If the on board LED is on, turn it off and visa versa. */
	if( P1 & ucLED_BIT )
	{
		P1 &= ~ucLED_BIT;
	}
	else
	{
		P1 |= ucLED_BIT;
	}
}
/*-----------------------------------------------------------*/

/*
 * See the documentation at the top of this file. 
 */
static void vErrorChecks( void *pvParameters )
{
portSHORT sErrorHasOccurred = pdFALSE;
	
	/* Just to prevent compiler warnings. */
	( void ) pvParameters;
	
	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error.   The delay period depends on whether an error
	has ever been detected. */
	for( ;; )
	{
		if( sLatchedError == pdFALSE )
		{		
			/* No errors have been detected so delay for a longer period.  The
			on board LED will get toggled every mainNO_ERROR_FLASH_PERIOD ms. */
			vTaskDelay( mainNO_ERROR_FLASH_PERIOD );
		}
		else
		{
			/* We have at some time recognised an error in one of the demo
			application tasks, delay for a shorter period.  The on board LED
			will get toggled every mainERROR_FLASH_PERIOD ms. */
			vTaskDelay( mainERROR_FLASH_PERIOD );
		}

		
		
		/* Check the demo application tasks for errors. */

		if( sAreIntegerMathsTaskStillRunning() != pdTRUE )
		{
			sErrorHasOccurred = pdTRUE;
		}

		if( sArePollingQueuesStillRunning() != pdTRUE )
		{
			sErrorHasOccurred = pdTRUE;
		}

		if( sAreComTestTasksStillRunning() != pdTRUE )
		{
			sErrorHasOccurred = pdTRUE;
		}

		if( sAreSemaphoreTasksStillRunning() != pdTRUE )
		{
			sErrorHasOccurred = pdTRUE;
		}

		/* If an error has occurred, latch it to cause the LED flash rate to 
		increase. */
		if( sErrorHasOccurred == pdTRUE )
		{
			sLatchedError = pdTRUE;
		}

		/* Toggle the LED to indicate the completion of a check cycle.  The
		frequency of check cycles is dependent on whether or not we have 
		latched an error. */
		prvToggleOnBoardLED();
	}
}
/*-----------------------------------------------------------*/

/*
 * See the documentation at the top of this file.  Also see the standard FLOP
 * demo task documentation for the rationale of these tasks.
 */
static void vFLOPCheck1( void *pvParameters )
{
volatile portFLOAT fVal1, fVal2, fResult;

	( void ) pvParameters;

	for( ;; )
	{
		fVal1 = ( portFLOAT ) -1234.5678;
		fVal2 = ( portFLOAT ) 2345.6789;

		fResult = fVal1 + fVal2;
		if( ( fResult > ( portFLOAT )  1111.15 ) || ( fResult < ( portFLOAT ) 1111.05 ) )
		{
			mainLATCH_ERROR();
		}

		fResult = fVal1 / fVal2;
		if( ( fResult > ( portFLOAT ) -0.51 ) || ( fResult < ( portFLOAT ) -0.53 ) )
		{
			mainLATCH_ERROR();
		}
	}
}
/*-----------------------------------------------------------*/

/*
 * See the documentation at the top of this file.
 */
static void vFLOPCheck2( void *pvParameters )
{
volatile portFLOAT fVal1, fVal2, fResult;

	( void ) pvParameters;

	for( ;; )
	{
		fVal1 = ( portFLOAT ) -12340.5678;
		fVal2 = ( portFLOAT ) 23450.6789;

		fResult = fVal1 + fVal2;
		if( ( fResult > ( portFLOAT ) 11110.15 ) || ( fResult < ( portFLOAT ) 11110.05 ) )
		{
			mainLATCH_ERROR();
		}

		fResult = fVal1 / -fVal2;
		if( ( fResult > ( portFLOAT ) 0.53 ) || ( fResult < ( portFLOAT ) 0.51 ) )
		{
			mainLATCH_ERROR();
		}
	}
}
/*-----------------------------------------------------------*/

/*
 * See the documentation at the top of this file. 
 */
static void vRegisterCheck( void *pvParameters )
{
	( void ) pvParameters;

	for( ;; )
	{
		if( SP != portSTACK_START )
		{
			mainLATCH_ERROR();
		}

		_asm
			MOV ACC, ar0
		_endasm;

		if( ACC != 0 )
		{
			mainLATCH_ERROR();
		}

		_asm
			MOV ACC, ar1
		_endasm;

		if( ACC != 1 )
		{
			mainLATCH_ERROR();
		}
		_asm
			MOV ACC, ar2
		_endasm;

		if( ACC != 2 )
		{
			mainLATCH_ERROR();
		}
		_asm
			MOV ACC, ar3
		_endasm;

		if( ACC != 3 )
		{
			mainLATCH_ERROR();
		}
		_asm
			MOV ACC, ar4
		_endasm;

		if( ACC != 4 )
		{
			mainLATCH_ERROR();
		}
		_asm
			MOV ACC, ar5
		_endasm;

		if( ACC != 5 )
		{
			mainLATCH_ERROR();
		}
		_asm
			MOV ACC, ar6
		_endasm;

		if( ACC != 6 )
		{
			mainLATCH_ERROR();
		}
		_asm
			MOV ACC, ar7
		_endasm;

		if( ACC != 7 )
		{
			mainLATCH_ERROR();
		}

		if( DPL != 0xcd )
		{
			mainLATCH_ERROR();
		}

		if( DPH != 0xab )
		{
			mainLATCH_ERROR();
		}

		if( B != 0x01 )
		{
			mainLATCH_ERROR();
		}			
	}
}


