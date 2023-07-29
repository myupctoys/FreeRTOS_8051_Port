#	FreeRTOS V2.4.1 - Copyright (C) 2003, 2004 Richard Barry.
#
#	This file is part of the FreeRTOS distribution.
#
#	FreeRTOS is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	FreeRTOS is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with FreeRTOS; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	A special exception to the GPL can be applied should you wish to distribute
#	a combined work that includes FreeRTOS, without being obliged to provide
#	the source code for any proprietary components.  See the licensing section 
#	of http://www.FreeRTOS.org for full details of how and when the exception
#	can be applied.
#
#	***************************************************************************
#	See http://www.FreeRTOS.org for documentation, latest information, license 
#	and contact details.  Please ensure to read the configuration and relevant 
#	port sections of the online documentation.
#	***************************************************************************

CC=sdcc
DEBUG=--debug

TARGET=main

SOURCE=..\..\..\Source
PORTABLE=$(SOURCE)\portable\SDCC\Cygnal

COMMON=..\..\Common
FULL=$(COMMON)/Full
MINIMAL=$(COMMON)/Minimal

SRC	= \
$(SOURCE)\tasks.c \
$(PORTABLE)\port.c \
$(SOURCE)\queue.c \
$(SOURCE)\list.c \
$(PORTABLE)\portheap.c \
..\ParTest\ParTest.c \
$(FULL)\flash.c \
$(FULL)\print.c \
$(MINIMAL)\integer.c \
$(MINIMAL)\PollQ.c \
..\serial\serial.c \
$(MINIMAL)\comtest.c \
$(FULL)\semtest.c

CFLAGS=--model-large -I. -I$(COMMON)\include -I$(SOURCE)\include -DSDCC_CYGNAL $(DEBUG) --less-pedantic --xram-size 8448 --stack-auto --no-peep --int-long-reent --float-reent

# Define all object files.
OBJ = $(SRC:.c=.rel)

OBJ_NO_PATH = $(notdir $(OBJ))

all : clean $(TARGET)

$(TARGET) : ..\$(TARGET).c $(PORTABLE)\portmacro.h $(OBJ)
	$(CC) $(CFLAGS) ..\$(TARGET).c $(OBJ_NO_PATH)

%.rel : %.c $(PORTABLE)\portmacro.h
	$(CC) -c $(CFLAGS) $<

.PHONY : clean
	
clean : 
	rm -rf *.adb *.asm *.lst *.rel *.rst *.sym *.cdb *.map *.ihx *.mem *.lnk *.o $(TARGET)

