/*
   ----------------------
   ShotTimer.c
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

#include <pic.h>
#include <StandardDefines.h>


#include <lcd.h>
#include <delay.h>

// Configuration Bits
#if XTAL_FREQ == 4MHZ
__CONFIG( XT & WDTDIS & PWRTDIS & BORDIS & LVPDIS & DEBUGDIS & PROTECT & MCLRDIS );
#else if XTAL_FREQ == 12MHZ
__CONFIG( HS & WDTDIS & PWRTDIS & BORDIS & LVPDIS & DEBUGDIS & PROTECT & MCLRDIS );
#endif


// Microphone Settings
#define ADC_DELAY() NOP(); NOP(); NOP(); NOP(); NOP();  NOP(); NOP(); NOP(); NOP(); NOP();  NOP(); NOP(); NOP(); NOP(); NOP();
#define STARTING_SOUND 2400
#define DEFAULT_SOUND 200
#define INC_SOUND 100
#define MIN_SOUND 100
#define LESSTHAN_SOUND 400 //200
#define MAXTHAN_SOUND 350

// Modes
#define MODE_SLEEP      0
#define MODE_ON         1
#define MODE_PROG       2

// Port Info
#define IN_POWERSW              RB0
#define IN_FUNCSW               RB1
#define OUT_LBIPOWER    RB3
#define OUT_MICPOWER    RB2
#define OUT_LCDPOWER    RA2

// Timing Definitions
#define ACTION() DI(); G_Last_Action = G_TickCount; EI(); NOP();
#define TICK_SECOND 1000000
#define HALF_SECOND (TICK_SECOND/2)

// Read/Write Volatile
volatile DWORD G_TickCount;                     // Timer In Microseconds

DWORD G_Last_Action;
DWORD G_PowerSW_FP;

bank1 BYTE G_Setting;
bank1 BYTE G_MaxSetting;

bank1 BYTE G_Mode;

DWORD G_Seconds;
BYTE G_Minutes;

WORD G_Input;
BOOL G_ShotDuration;
DWORD G_ShotStart, G_ShotEnd;
WORD G_ShotCount;
WORD G_Highest;
BOOL G_Refresh;
DWORD G_Refresh_LastCheck;
DWORD G_Setting_Fastest;
DWORD G_Setting_Slowest;
WORD G_High;
WORD G_Low;
WORD G_Idle;
WORD G_UHigh;
BOOL G_HighShot;

BOOL G_LBI;

bank1 float G_ShotList[25];


/* @Desc: Interrupt
   @Return:
 */
void interrupt INT(void)
{
    if (TMR0IF) {
#if XTAL_FREQ == 4MHZ
        G_TickCount += 255;     // 4MHZ Oscillator
#else if XTAL_FREQ == 12MHZ
        G_TickCount += 85;              // 12MHZ Oscillator
#endif
        TMR0 -= 255;
        TMR0IF = 0;
    }

    if (INT0IF) {
        NOP();
        INT0IF = 0;
    }

    KICKDOG();
}


/* @Desc: Reset All Of The Timing Values
   @Return:
 */
void ResetSettings(void)
{
    DI();
    G_Setting_Fastest = 0;
    G_Setting_Slowest = 0;
    G_ShotCount = 0;
    G_Minutes = 0;
    G_Seconds = 0;

    G_Refresh_LastCheck = 0;

    G_Highest = 0;
    G_HighShot = FALSE;

    // Timing
    G_TickCount = 0;
    G_ShotStart = 0;
    G_ShotEnd = 0;
    G_Last_Action = 0;
    G_PowerSW_FP = 0;
    EI();
    NOP();
}


/* @Desc: Check The Microphone, Analog Ditial Conversion Routine
   @Return:
 */
void CheckMic(void)
{
    DWORD endtime;
    DWORD rof;

    ADC_DELAY();
    ADGO = 1;
    ADC_DELAY();
    while (ADGO == 1) NOP();
    ADC_DELAY();

    G_Input = ( ADRESL | (ADRESH<<8) );

    if (G_Input > G_Highest)
        G_Highest = G_Input;

    if (G_Mode == MODE_ON) {
        if (G_ShotDuration == FALSE) {
            if (G_Input >= G_UHigh) {
                G_ShotDuration = TRUE;

                if (G_ShotStart == 0) {
                    DI();
                    G_ShotStart = G_TickCount;
                    EI();
                    NOP();

                    endtime = G_ShotStart;
                    G_ShotEnd = 0;
                } else {
                    DI();
                    G_ShotEnd = G_TickCount;
                    EI();
                    NOP();

                    rof = (G_ShotEnd-G_ShotStart);

                    if (G_ShotCount && (G_ShotCount <= 25))
                        G_ShotList[G_ShotCount-1] = rof;

                    G_ShotStart = G_ShotEnd;
                    G_ShotEnd = 0;

                    if ((rof < G_Setting_Fastest) || !G_Setting_Fastest)
                        G_Setting_Fastest = rof;

                    if ((rof > G_Setting_Slowest) || !G_Setting_Slowest)
                        G_Setting_Slowest = rof;
                }
            }
        } else {
            if (G_Highest <= G_Low) {
                DI();
                if (G_TickCount >= endtime+33333) {
                    EI();
                    NOP();
                    G_ShotDuration = FALSE;
                    G_ShotCount++;
                    G_Refresh = TRUE;
                    DI();
                }
                EI();
                NOP();
            } else {
                G_Highest = 0;

                DI();
                endtime = G_TickCount;
                EI();
                NOP();
            }
        }
    }
}


/* @Desc: Perform Boot Sequence, Power Board, Init Timers, etc
   @Return:
 */
void StartUp(void)
{
    WORD x;

#ifdef LBI
// LBI
    DI();
    EI();
    NOP();
    TRISB3 = 0;
    RB3 = 1;                                // Turn LBI On
    DelayMs(255);
    CHS2 = 0;
    CHS1 = 0;
    CHS0 = 1;
    TRISA1 = 1;
    CVRCON = 0b10001111;
    CMCON =  0b00000010;
    DelayMs(255);
    if (C2OUT == 1) {
        G_LBI = TRUE;
    } else {
        G_LBI = FALSE;
    }
#endif

// Port Setup
    DI();
    TRISB = 0b00000011;             // 2 Switch Inputs
    TRISA = 0b00000001;             // 1 Analog Input
    PORTB = 0b00000000;     // PortB Settings
    PORTA = 0b00000000;             // PortA Settings
    OUT_MICPOWER = 1;               // Power The Microphone
    OPTION =        0b00001000;     // RBPU | INTEDG | T0CS | *PSA | PS2 | PS1 | PS0
    ADCON0 =        0b00000001; // ADCS1 | ADCS0 | CHS2 | CHS1 | CHS0 | GO/DONE | N | *ADON
    ADCON1 =        0b00001110; // ADFM | ADCS2 | VCFG1 | VCFG0 | N | N | N | N
    OSCCON =        0b01100000; // N | *IRCF2 | *IRCF1 | IRCF0 | N | IOFS | N | N
    INTCON =        0b01100000; // GIE | *PEIE | *TMR0IE | INTE | RBIE | TMR0IF | INTF | RBIF
    PIE1 =          0b00000000; // N | ADIE
    PIR1 =          0b00000000; // N | ADIF
    ANSEL =         0b00000001; // N | ANS6 | ANS5 | ANS4 | ANS3 | ANS2 | ANS1 | ANS0
    CVRCON =        0b00000000;
    CMCON =         0b00000111;
    EI();
    NOP();

    if (G_Mode == MODE_SLEEP) {
        ADC_DELAY();
        ADGO = 1; // Start Conversion

        // Done With Initial Bootup
        ResetSettings();

        // Intialize LCD
        OUT_LCDPOWER = 1;       // Power The LCD Screen
        LCD_RS = 0;
        LCD_EN = 0;

        DelayMs(15);    // wait 15mSec after power applied,
        RB7 = 0;
        RB6 = 0;
        RB5 = 1;
        RB4 = 1;
        LCD_STROBE();
        DelayMs(5);
        LCD_STROBE();
        DelayUs(200);
        LCD_STROBE();
        DelayUs(200);
        RB7 = 0;
        RB6 = 0;
        RB5 = 1;
        RB4 = 0;
        LCD_STROBE();

        lcd_write(0x28); // Set interface length
        lcd_write(0xF); // Display On, Cursor On, Cursor Blink
        lcd_write(12);  // Hide Cursor
        lcd_clear();    // Clear screen
        lcd_write(0x6); // Set entry Mode
        lcd_clear();

        // Read EEPROM
        if (eeprom_read(0x00) != VER) {
            G_High = DEFAULT_SOUND;
        } else {
            G_High = MIN_SOUND+(INC_SOUND*eeprom_read(0x01));
        }

// Do Bootup
#ifdef LBI
        if (G_LBI == TRUE) {
            lcd_goto(0); // select first line
            lcd_puts("Low Battery ------");
            lcd_goto(0x40); // Select second line
            lcd_puts("Change The Battery");
            DelayMs(255);
            DelayMs(255);
            DelayMs(255);
            DelayMs(255);
        }
#endif

        /*
                if (RB1 == 0)
                {
                        lcd_goto(0);	// select first line
                        lcd_puts("Programming Mode");
                        lcd_goto(0x40);	// Select second line
                        lcd_puts("_-_-_-_-_-_-_-_-_-");
                        G_Mode = MODE_PROG;
                        G_Setting = G_High/INC_SOUND; //(G_High-MIN_SOUND/INC_SOUND;
                        G_MaxSetting = 10;
                }
                else
                {
         */
        lcd_goto(0);    // select first line
        lcd_puts("  ShotTimer(TM) ");
        lcd_goto(0x40); // Select second line
        lcd_puts(LCD_VER);
        DelayMs(255);
        DelayMs(255);


        // Calculate The High And Low Settings
        G_High += STARTING_SOUND;

        // Set High And Low
        G_UHigh = G_High;
        G_Low = (WORD) G_UHigh-LESSTHAN_SOUND;

        G_Mode = MODE_ON;
        G_Setting = 1;
        G_MaxSetting = 5;
        /*
                }
         */
        G_Refresh = TRUE;
    }

    G_ShotCount = 0;
}


/* @Desc: Wait For A Amount Of Time
   @Return:
 */
void ShutDown(void)
{
    lcd_clear();

    // Save To EEPROM
    if (G_Mode == MODE_PROG) {
        eeprom_write(0x00, VER);
        eeprom_write(0x01, G_Setting);
    }

    while(IN_POWERSW == 0) NOP();

// Port Setup
    DI();
    OPTION =        0b00001000;     // RBPU | INTEDG | T0CS | *PSA | PS2 | PS1 | PS0
    ADCON0 =        0b00000000; // ADCS1 | ADCS0 | CHS2 | CHS1 | CHS0 | GO/DONE | N | ADON
    ADCON1 =        0b00000000; // ADFM | ADCS2 | VCFG1 | VCFG0 | N | N | N | N
    OSCCON =        0b00000001;     // N | IRCF2 | IRCF1 | IRCF0 | N | IOFS | SCS1 | SCS0 (TESTING)
    INTCON =        0b00010000;     // GIE | PEIE | TMR0IE | INTE | RBIE | TMR0IF | INTF | RBIF
    PIE1 =          0b00000000; // N | ADIE
    PIR1 =          0b00000000; // N | ADIF
    ANSEL =         0b00000000; // N | ANS6 | ANS5 | ANS4 | ANS3 | ANS2 | ANS1 | ANS0
    CVRCON =        0b00000000;
    CMCON =         0b00000111;

    /* test 1
       PORTB = 0x01;
       PORTA = 0x00;
       DelayMs(50); //testing
     */

// test 2
    /*
       TRISB = 0x01;
       TRISA = 0x00;
       PORTB = 0x01;
       PORTA = 0x00;
       DelayMs(50); //testing
       TRISA1 = 1; // Set LBI Input Pin As a Input

       INTEDG = 0; // Interrupt When They Press A Button
       RBPU = 0; // enable //1;	// Disable PortB Internal Pullup Before Going To Sleep
     */

// test 3
    /*
       INTEDG = 0; // Interrupt When They Press A Button
       RBPU = 0;	// Enable PortB Internal Pullup Before Going To Sleep
       TRISB = 0x01;
       TRISA = 0x00;
       PORTB = 0x01;
       PORTA = 0x00;
       DelayMs(50); //testing
       TRISA1 = 1; // Set LBI Input Pin As a Input
     */

// test 4
    INTEDG = 0; // Interrupt When They Press A Button
    RBPU = 0; // Enable PortB Internal Pullup Before Going To Sleep
//TRISB = 0x01;
//TRISA = 0x00;
    PORTB = 0x01;
    PORTA = 0x00;
    DelayMs(50); //testing

    EI();
    NOP();
    DelayMs(50);

    SLEEP();

    TRISB = 0x01; // testing
    TRISA = 0x00; // testing
    PORTB = 0x01; // testing
    PORTA = 0x00; // testing

    //TRISA2 = 0; // TESTING!! MUST SET FOR LED TO COME ON
    //RA2 = 1; // LCD on TESTING
    DelayMs(255); // TESTING
    DelayMs(255); // TESTING

    RBPU = 0; // Enable PortB Internal Pullup Before Going To Sleep
    PEIE = 0; // Disable Perifial Until StartUp

    G_Mode = MODE_SLEEP;
}


/* @Desc: MAIN!!!!
   @Return: 0 If Successful
 */
void main()
{
    static BOOL funcpress = 0;

    // Default ShutDown
    DelayMs(10);
    G_Mode = MODE_ON;
    StartUp();
    ShutDown();

    while(TRUE) {
        NOP();

        // PowerButton (TurnOn/TurnOff)
        if (IN_POWERSW == 0) {
            ACTION();

            if (G_Mode == MODE_SLEEP) {
                // StartUp
                StartUp();
            } else {
                if (G_PowerSW_FP == NULL) {
                    // Clear Settings
                    if (G_Mode == MODE_ON) {
                        if (G_Setting == 1) {
                            G_Setting_Fastest = 0;

                            G_Setting_Slowest = 0;
                            G_ShotCount = 0;
                            G_ShotStart = 0;
                        } else if (G_Setting == 2) {
                            G_Setting_Slowest = 0;

                            G_Setting_Fastest = 0;
                            G_ShotCount = 0;
                            G_ShotStart = 0;
                        } else if (G_Setting == 3) {
                            G_Setting_Fastest = 0;
                            G_Setting_Slowest = 0;

                            G_ShotCount = 0;
                            G_ShotStart = 0;
                        } else if (G_Setting == 4) {
                            G_ShotCount = 0;

                            G_Setting_Fastest = 0;
                            G_Setting_Slowest = 0;
                            G_ShotStart = 0;
                        } else if (G_Setting == 5) {
                            G_Minutes = 0;
                            G_Seconds = 0;
                            DI();
                            G_Refresh_LastCheck = G_TickCount;
                            G_Seconds = G_Refresh_LastCheck;
                            EI();
                            NOP();
                        } else if (G_Setting >= G_MaxSetting+1) {
                            G_Setting = 1;                         // First Setting

                            G_Setting_Fastest = 0;

                            G_Setting_Slowest = 0;
                            G_ShotCount = 0;
                            G_ShotStart = 0;

                        }
                        DI();
                        G_HighShot = FALSE;
                        G_UHigh = G_High;
                        G_Low = (WORD) G_UHigh-LESSTHAN_SOUND;
                        EI();
                        NOP();
                    }
                    G_Refresh = TRUE;

                    DI();
                    G_PowerSW_FP = G_TickCount;
                    EI();
                    NOP();
                }

                DI();
                if (G_TickCount > G_PowerSW_FP+(TICK_SECOND*2)) {
                    EI();
                    NOP();
                    G_PowerSW_FP = NULL;

                    // Save Settings N ShutDown
                    ShutDown();
                }
                EI();
                NOP();
            }
        } else {
            G_PowerSW_FP = NULL;

            if (G_Mode == MODE_SLEEP) { // If Power Button Released After PowerDown Go Back To Sleep
                ShutDown();
            }
        }

        // Increment Game Timer
        DI();
        if (G_TickCount-G_Seconds > 60000000) {
            EI();
            NOP();
            G_Minutes += 1;
            G_Refresh = TRUE;
            DI();
            G_Seconds = G_TickCount;
            G_Refresh_LastCheck = G_TickCount; // Calibrate Update
        }
        EI();
        NOP();
        if (G_Setting == 5) {
            DI();
            if (G_TickCount > G_Refresh_LastCheck+1000000) {
                EI();
                NOP();
                G_Refresh = TRUE;
                DI();
                G_Refresh_LastCheck = G_TickCount;
            }
            EI();
            NOP();
        }

        // Function Switch (Increment)
        if (IN_FUNCSW == 0) {
            ACTION();

            if (!funcpress) {
                funcpress = 1;

                G_Setting++;

                if (G_ShotCount) {
                    if ((G_Setting > G_MaxSetting+(G_ShotCount-1)) || G_Setting > G_MaxSetting+25)
                        G_Setting = 1;
                } else {
                    if (G_Setting > G_MaxSetting)
                        G_Setting = 1;
                }

                G_Refresh = TRUE;
            }
        } else {
            funcpress = 0;
        }


        if (G_Refresh) {
            if (G_Mode == MODE_ON) {
                if (G_Setting == 1) {
                    lcd_clear();
                    lcd_goto(0);    // select first line
                    lcd_puts("Fastest MS/BPS");
                    lcd_goto(0x40); // Select second line
                    lcd_puts("M:");
                    lcd_writefloat((float)(G_Setting_Fastest/1000.0f));
                    lcd_puts("  B:");
                    lcd_writefloat((float)(1000000.0f/G_Setting_Fastest));
                } else if (G_Setting == 2) {
                    lcd_clear();
                    lcd_goto(0);    // select first line
                    lcd_puts("Slowest BPS");
                    lcd_goto(0x40); // Select second line
                    lcd_puts("BPS:");
                    lcd_writefloat((float)(1000000.0f/G_Setting_Slowest));
                } else if (G_Setting == 3) {
                    lcd_clear();
                    lcd_goto(0);    // select first line
                    lcd_puts("Average BPS");
                    lcd_goto(0x40); // Select second line
                    lcd_puts("BPS:");
                    lcd_writefloat((float)(((1000000.0f/G_Setting_Fastest)+(1000000.0f/G_Setting_Slowest))/2));
                } else if (G_Setting == 4) {
                    lcd_clear();
                    lcd_goto(0);    // select first line
                    lcd_puts("ShotCounter");
                    lcd_goto(0x40); // Select second line
                    lcd_puts("Shots:");
                    lcd_writeint((BYTE)G_ShotCount, STYLE_STD);
                } else if (G_Setting == 5) {
                    lcd_clear();
                    lcd_goto(0);    // select first line
                    lcd_puts("Game Timer");
                    lcd_goto(0x40); // Select second line
                    lcd_writeint(G_Minutes, STYLE_STD);
                    lcd_puts(":");
                    lcd_writeint((G_TickCount-G_Seconds)/1000000, STYLE_CLK);
                } else {
                    lcd_clear();
                    lcd_goto(0);    // select first line
                    lcd_puts("Shot ");
                    lcd_writeint(G_Setting-(G_MaxSetting), STYLE_STD);
                    lcd_puts(" MS/BPS");
                    lcd_goto(0x40); // Select second line
                    lcd_puts("M:");
                    lcd_writefloat((float)(G_ShotList[G_Setting-(G_MaxSetting+1)]/1000.0f));
                    lcd_puts(" B:");
                    lcd_writefloat((float)(1000000.0f/G_ShotList[G_Setting-(G_MaxSetting+1)]));
                }
            } else if (G_Mode == MODE_PROG) {
                lcd_clear();
                lcd_goto(0);            // select first line
                lcd_puts("Programming Mode");
                lcd_goto(0x40);         // Select second line
                lcd_puts("Mic Threshold:");
                lcd_writeint(G_Setting, STYLE_STD);
            }
            G_Refresh = FALSE;
        }

        // 60min/10min Power SAVE
        DI();
        if (G_TickCount > G_Last_Action+(TICK_SECOND*60*30)) {
            EI();
            NOP();
            // Record Action
            ACTION();
            // ShutDown
            ShutDown();
        }
        EI();
        NOP();

        if ((G_Mode == MODE_ON) && OUT_MICPOWER)
            CheckMic();

        NOP();
    }
}
