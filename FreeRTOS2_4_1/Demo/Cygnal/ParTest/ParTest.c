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
#include <c8051f120.h>

#include "projdefs.h"
#include "task.h"
#include "portable.h"
#include "partest.h"

#define partstPUSH_PULL			( ( unsigned portCHAR ) 0xff )
#define partstALL_OUTPUTS_OFF	( ( unsigned portCHAR ) 0xff )

/* LED to output is dependent on how the LED's are wired. */
#define partstOUTPUT_0			( ( unsigned portCHAR ) 0x02 )
#define partstOUTPUT_1			( ( unsigned portCHAR ) 0x08 )
#define partstOUTPUT_2			( ( unsigned portCHAR ) 0x20 )
#define partstOUTPUT_3			( ( unsigned portCHAR ) 0x01 )
#define partstOUTPUT_4			( ( unsigned portCHAR ) 0x04 )
#define partstOUTPUT_5			( ( unsigned portCHAR ) 0x10 )
#define partstOUTPUT_6			( ( unsigned portCHAR ) 0x40 )
#define partstOUTPUT_7			( ( unsigned portCHAR ) 0x80 )

/*-----------------------------------------------------------
 * Simple parallel port IO routines.
 *-----------------------------------------------------------*/

void vParTestInitialise( void )
{
unsigned portCHAR ucOriginalSFRPage;

	/* Remember the SFR page before it is changed so it can get set back
	before the function exits. */
	ucOriginalSFRPage = SFRPAGE;

	/* Setup the SFR page to access the config SFR's. */
	SFRPAGE = CONFIG_PAGE;

	/* Set the on board LED to push pull. */
	P3MDOUT |= partstPUSH_PULL;

	/* Return the SFR page. */
	SFRPAGE = ucOriginalSFRPage;

	P3 = partstALL_OUTPUTS_OFF;
}
/*-----------------------------------------------------------*/

void vParTestSetLED( unsigned portCHAR ucLED, portCHAR cValue )
{
portCHAR cError = ( portCHAR ) pdFALSE;

	vTaskSuspendAll();
	{
		if( cValue == pdFALSE )
		{
			switch( ucLED )
			{
				case 0	:	P3 |= partstOUTPUT_0;
							break;
				case 1	:	P3 |= partstOUTPUT_1;
							break;
				case 2	:	P3 |= partstOUTPUT_2;
							break;
				case 3	:	P3 |= partstOUTPUT_3;
							break;
				case 4	:	P3 |= partstOUTPUT_4;
							break;
				case 5	:	P3 |= partstOUTPUT_5;
							break;
				case 6	:	P3 |= partstOUTPUT_6;
							break;
				case 7	:	P3 |= partstOUTPUT_7;
							break;
				default	:	/* There are no other LED's wired in. */
							cError = ( portCHAR ) pdTRUE;
							break;
			}
		}
		else
		{
			switch( ucLED )
			{
				case 0	:	P3 &= ~partstOUTPUT_0;
							break;
				case 1	:	P3 &= ~partstOUTPUT_1;
							break;
				case 2	:	P3 &= ~partstOUTPUT_2;
							break;
				case 3	:	P3 &= ~partstOUTPUT_3;
							break;
				case 4	:	P3 &= ~partstOUTPUT_4;
							break;
				case 5	:	P3 &= ~partstOUTPUT_5;
							break;
				case 6	:	P3 &= ~partstOUTPUT_6;
							break;
				case 7	:	P3 &= ~partstOUTPUT_7;
							break;
				default	:	/* There are no other LED's wired in. */
							break;
			}
		}
	}
	cTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vParTestToggleLED( unsigned portCHAR ucLED )
{
unsigned portCHAR ucBit;
portCHAR cError = ( portCHAR ) pdFALSE;

	vTaskSuspendAll();
	{
		switch( ucLED )
		{
			case 0	:	ucBit = partstOUTPUT_0;
						break;
			case 1	:	ucBit = partstOUTPUT_1;
						break;
			case 2	:	ucBit = partstOUTPUT_2;
						break;
			case 3	:	ucBit = partstOUTPUT_3;
						break;
			case 4	:	ucBit = partstOUTPUT_4;
						break;
			case 5	:	ucBit = partstOUTPUT_5;
						break;
			case 6	:	ucBit = partstOUTPUT_6;
						break;
			case 7	:	ucBit = partstOUTPUT_7;
						break;
			default	:	/* There are no other LED's wired in. */
						cError = ( portCHAR ) pdTRUE;
						break;
		}

		if( cError != ( portCHAR ) pdTRUE )
		{
			if( P3 & ucBit )
			{
				P3 &= ~ucBit;
			}
			else
			{
				P3 |= ucBit;
			}
		}
	}
	cTaskResumeAll();
}


