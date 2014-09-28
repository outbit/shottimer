/*
   ----------------------
   standarddefines.h
   ----------------------
   Created: 04/10/06
   Updated: 06/27/06
   Version: 1.2.1
   ----------------------
   Compiler: PICC 9.40 C Compiler From HI-TECH
   Assembler: PICC 9.40 Assembler From HI-TECH
   Linker: PICC Linker From HI-TECH
   IDE: MPLAB IDE v6.62
   ----------------------
   Author: David Lee Whiteside
   Company: Hyperformance Paintball LLC
   Project: ShotTimer(TM)
   Target MCU: PIC16F88 From Microchip
   ----------------------
   The ShotTimer(TM) software is (C) 2004-2009 Whiteside Solutions LLC.
   This program is free software; you may redistribute and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; Version 3.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License v3.0 for more details at http://www.gnu.org/licenses/gpl-3.0.html
 */
#ifndef _STANDARD_DEFINES_H_
#define  _STANDARD_DEFINES_H_

#define LBI // For Low Battery Indicator
#define XTAL_FREQ 12MHZ // 12mhz Oscillator
#define VER 0x04
#define LCD_VER "Firmware: v1.2.1"
/*
   0x01 - Version 1.0 - interrupt based microphone
   0x02 - Version 1.1 - 4mhz oscillator, lbi added
   0x03 - Version 1.2 - 12 mhz oscillator
   0x04 - Version 1.2.1 - First Released
 */

// Standard Defines
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL  0
#endif

// Standard Typedefs
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#ifndef BOOL
typedef BYTE BOOL;
#endif

// PIC Macros
#ifndef NOP   // Null Operation
#define NOP()    asm ("nop")
#endif
#ifndef EI    // Global Interrupt Enable
#define EI()     (GIE = TRUE)
#endif
#ifndef DI    // Global Interrupt Disable
#define DI()     (GIE = FALSE)
#endif
#ifndef SLEEP // Sleep Mode
#define SLEEP()   asm ("sleep")
#endif
#ifndef CLRWDT // Control Watch Dog Timer
#define CLRWDT()  asm ("clrwdt")
#endif
#ifndef KICKDOG // Control Watch Dog Timer
#define KICKDOG()  //CLRWDT()
#endif

#endif  //_STANDARD_DEFINES_H_
