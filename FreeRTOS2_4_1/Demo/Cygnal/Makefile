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
CFLAGS=--model-large -I. -I..\Common\include -I..\include -I..\..\Source\include -DSDCC_CYGNAL $(DEBUG) --less-pedantic --xram-size 8448 --stack-auto --no-peep --int-long-reent --float-reent

SRC	= \
..\..\Source\tasks.c \
..\..\Source\portable\SDCC\Cygnal\port.c \
..\..\Source\queue.c \
..\..\Source\list.c \
..\..\Source\portable\SDCC\Cygnal\portheap.c \
ParTest\ParTest.c \
..\Common\Full\flash.c \
..\Common\Full\print.c \
..\Common\Minimal\integer.c \
..\Common\Minimal\PollQ.c \
serial\serial.c \
..\Common\Minimal\comtest.c \
..\Common\Full\semtest.c

# Define all object files.
OBJ = $(SRC:.c=.rel)

OBJ_NO_PATH = $(notdir $(OBJ))

ODIR = .\build

######################################
# THIS VERSION WILL ONLY BUILD FILES THAT HAVE CHANGED, BUT MAY HAVE A DANGEROUS
# COMMAND LINE.  IT WORKS FINE UNDER WINDOWS, BUT I HAVE COMMENTED IT OUT IN
# CASE IT CAUSES PROBLEMS ON OTHER SYSTEMS.

#main : main.c Makefile ..\..\Source\portable\SDCC\Cygnal\portmacro.h $(OBJ)
#	$(CC) $(CFLAGS) main.c $(OBJ)

#%.rel : %.c Makefile ..\..\Source\portable\SDCC\Cygnal\portmacro.h
#	$(CC) -c $(CFLAGS) -o$< $<

######################################
# INSTEAD OF THE FOUR LINES ABOVE, THIS VERSION CAN BE USED BUT WILL CAUSE ALL
# FILES TO BUILD EVERY TIME.

main : main.c Makefile ..\..\Source\portable\SDCC\Cygnal\portmacro.h $(OBJ)
	$(CC) $(CFLAGS) main.c $(OBJ_NO_PATH)

%.rel : %.c Makefile ..\..\Source\portable\SDCC\Cygnal\portmacro.h
	$(CC) -c $(CFLAGS) $<

.PHONY : clean
	
clean : 
	rm -f $(SRC) *.adb
	rm -f $(SRC) *.asm
	rm -f $(SRC) *.lst
	rm -f $(SRC) *.rel
	rm -f $(SRC) *.rst
	rm -f $(SRC) *.sym
	rm -f $(SRC) *.cdb
	rm -f $(SRC) *.map
	rm -f $(SRC) *.ihx
	rm -f $(SRC) *.mem
	rm -f $(SRC) *.lnk
