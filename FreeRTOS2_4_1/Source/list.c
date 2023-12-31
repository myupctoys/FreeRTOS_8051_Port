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
Changes from V1.2.0

	+ Removed the volatile modifier from the function parameters.  This was
	  only ever included to prevent compiler warnings.  Now warnings are 
	  removed by casting parameters where the calls are made.

	+ prvListGetOwnerOfNextEntry() and prvListGetOwnerOfHeadEntry() have been
	  removed from the c file and added as macros to the h file.

	+ usNumberOfItems has been added to the list structure.  This removes the
	  need for a pointer comparison when checking if a list is empty, and so
	  is slightly faster.

	+ Removed the NULL check in vListRemove().  This makes the call faster but
	  necessitates any application code utilising the list implementation to
	  ensure NULL pointers are not passed.

Changes from V2.0.0

	+ Double linked the lists to allow faster removal item removal.
*/

#include <stdlib.h>
#include "projdefs.h"
#include "portable.h"
#include "list.h"

/*-----------------------------------------------------------
 * PUBLIC LIST API documented in list.h
 *----------------------------------------------------------*/

void vListInitialise( xList *pxList )
{
	/* The list structure contains a list item which is used to mark the 
	end of the list.  To initialise the list the list end is inserted
	as the only list entry. */
	pxList->pxHead = &( pxList->xListEnd );
	pxList->pxIndex = pxList->pxHead;

	/* The list end value is the highest possible value in the list to
	ensure it remains at the end of the list. */
	pxList->xListEnd.xItemValue = portMAX_DELAY;

	/* The list end next and previous pointers point to itself so we know
	when the list is empty. */
	pxList->xListEnd.pxNext = &( pxList->xListEnd );
	pxList->xListEnd.pxPrevious = &( pxList->xListEnd );

	/* The list head will never get used and has no owner. */
	pxList->xListEnd.pvOwner = NULL;

	/* Make sure the marker items are not mistaken for being on a list. */
	vListInitialiseItem( ( xListItem * ) &( pxList->xListEnd ) );

	pxList->usNumberOfItems = ( unsigned portSHORT ) 0;
}
/*-----------------------------------------------------------*/

void vListInitialiseItem( xListItem *pxItem )
{
	/* Make sure the list item is not recorded as being on a list. */
	pxItem->pvContainer = NULL;
}
/*-----------------------------------------------------------*/

void vListInsertEnd( xList *pxList, xListItem *pxNewListItem )
{
volatile xListItem * pxIndex;

	/* Insert a new list item into pxList, but rather than sort the list, 
	makes the new list item the last item to be removed by a call to 
	pvListGetOwnerOfNextEntry.  This means it has to be the item pointed to by
	the pxIndex member. */
	pxIndex = pxList->pxIndex;

	pxNewListItem->pxNext = pxIndex->pxNext;
	pxNewListItem->pxPrevious = pxList->pxIndex;
	pxIndex->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
	pxIndex->pxNext = ( volatile xListItem * ) pxNewListItem;
	pxList->pxIndex = ( volatile xListItem * ) pxNewListItem;

	/* Remember which list the item is in. */
	pxNewListItem->pvContainer = ( void * ) pxList;

	( pxList->usNumberOfItems )++;
}
/*-----------------------------------------------------------*/

void vListInsert( xList *pxList, xListItem *pxNewListItem )
{
volatile xListItem *pxIterator;
register portTickType xValueOfInsertion;

	/* Insert the new list item into the list, sorted in ulListItem order. */
	xValueOfInsertion = pxNewListItem->xItemValue;

	/* If the list already contains a list item with the same item value then
	the new list item should be placed after it.  This ensures that TCB's which
	are stored in ready lists (all of which have the same ulListItem value)
	get an equal share of the CPU.  However, if the xItemValue is the same as 
	the back marker the iteration loop below will not end.  This means we need
	to guard against this by checking the value first and modifying the 
	algorithm slightly if necessary. */
	if( xValueOfInsertion == portMAX_DELAY )
	{
		for( pxIterator = pxList->pxHead; pxIterator->pxNext->xItemValue < xValueOfInsertion; pxIterator = pxIterator->pxNext )
		{
			/* There is nothing to do here, we are just iterating to the 
			wanted insertion position. */
		}
	}
	else
	{
		for( pxIterator = pxList->pxHead; pxIterator->pxNext->xItemValue <= xValueOfInsertion; pxIterator = pxIterator->pxNext )
		{
			/* There is nothing to do here, we are just iterating to the 
			wanted insertion position. */
		}
	}

	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
	pxNewListItem->pxPrevious = pxIterator;
	pxIterator->pxNext = ( volatile xListItem * ) pxNewListItem;

	/* Remember which list the item is in.  This allows fast removal of the
	item later. */
	pxNewListItem->pvContainer = ( void * ) pxList;

	( pxList->usNumberOfItems )++;
}
/*-----------------------------------------------------------*/

void vListRemove( xListItem *pxItemToRemove )
{
xList * pxList;

	pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
	pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
	
	/* The list item knows which list it is in.  Obtain the list from the list
	item. */
	pxList = ( xList * ) pxItemToRemove->pvContainer;

	/* Make sure the index is left pointing to a valid item. */
	if( pxList->pxIndex == pxItemToRemove )
	{
		pxList->pxIndex = pxItemToRemove->pxPrevious;
	}

	pxItemToRemove->pvContainer = NULL;
	( pxList->usNumberOfItems )--;
}
/*-----------------------------------------------------------*/

