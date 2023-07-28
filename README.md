# 8051 Port with FreeRTOS 2_4_1 with SDCC 2_5_0
FreeRTOS 2_4_1 with matched SDCC 2_5_0 compiler

Caveat, don't expect support from the SDCC or the FreeRTOS development teams given this is now 18 
years old. Only intended as a starting point for the 8051 port. As with others, all I wanted 
is a starting point. Hopefully this helps someone else out.

Went through the process of trying to port a newer version of the FreeRTOS Cygnal port to
a newer version of SDCC. Got a fair way through only to be met with a parser bug in SDCC that
apparently was introduced to SDCC a long time ago. Just dumb luck the Silabs/Cygnal port stumbles into it,
presumably not enough of an issue that it causes much of a problem elsewhere.

Only reason I figured this path out was becasue I played with 8051's FreeRTOS and SDCC in the very early 
2000's and know it worked at that point in time. Just a case of ploughing through the half a dozen versions 
I had floating around to find the latest combination that worked.

https://www.freertos.org/portcygn.html

Have been able to confirm FreeRTOS V2.4.1 compiles with SDCC V2.5.0 May 8 2005 without error, 
zero modifications to the source and the Makefile as is.  I did add a clean to the Makefile 
to get rid of the clutter after a compile.

End up with an Intel hex file. "main.ihx" file placed in the root folder directory for reference. 
Original FreeRTOS project folder structure retained but removed the other ports in the project. They still
exist in the zip.

Just need the path added for your Make and SDCC binary. Update the SDDC_Path.bat for your local instance. 
For completeness, GNU Make version used was 64 bit 4.3 on W10 64 bit.

https://www.cygwin.com

There was a documented issue with cygwin1.dll in relation to SDCC. Exact nature of the problem escapes me,
and it may not be needed. Have included the dll in case it makes a difference given I have the new dll version 
on the three locations I've tested the FreeRTOS port out on. If you can't compile properly put/replace 
the existing dll in the Cygwin64 bin directory.

Don’t know if the FreeRTOS binary works, just that it compiles. At least it builds confidence and should only be
treated as a starting point. 

#1 Clone or Zip this Repo locally to your PC.<BR>
#2 Unzip the SDCC zip to a likely place on your HDD (preferable without spaces in the directory names).<BR>
#3 Install GNU make (not included), I used GNU make 4.3 in Cygwin 64 bit on W10.<BR>
#4 Modify the SDCC_Path.bat file in the Cygnal demo directory to point to SDCC and your Make binary "bin" folders.<BR>
#5 Run the SDCC_Path.bat batch file.<BR>

Then command line "make" from the Cygnal Folder. "make clean" cleans out the clutter.
Should see a screen as below.<BR><BR>
![Cygnal_Compile](https://github.com/myupctoys/RTOS/assets/5317221/b34de0b9-3a5d-415b-876b-9609daa3bad2)

