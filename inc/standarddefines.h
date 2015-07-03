/*
   ----------------------
   standarddefines.h
   ----------------------
   Created: 04/10/06
   Updated: 07/03/15
   Version: 1.3.0
   ----------------------
   Compiler: PICC 9.40 C Compiler From HI-TECH
   Assembler: PICC 9.40 Assembler From HI-TECH
   Linker: PICC Linker From HI-TECH
   IDE: MPLAB IDE v6.62
   ----------------------
   Author: David Lee Whiteside
   Project: ShotTimer(TM)
   Target MCU: PIC16F88 From Microchip
   ----------------------
   Copyright (c) 2004-2015 David Whiteside

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
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
