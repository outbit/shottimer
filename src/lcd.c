
/*
 *	LCD interface example
 *	Uses routines from delay.c
 *	This code will interface to a standard LCD controller
 *	like the Hitachi HD44780. It uses it in 4 bit mode, with
 *	the hardware connected as follows (the standard 14 pin
 *	LCD connector is used):
 *
 *	PORTD bits 4-7 are connected to the LCD data bits 4-7 (high nibble)
 *	PORTA bit 4 is connected to the LCD RS input (register select)
 *	PORTA bit 3 is connected to the LCD EN bit (enable)
 *
 *	To use these routines, set up the port I/O (TRISA, TRISD) then
 *	call lcd_init(), then other routines as required.
 *
 */
#include <pic.h>
#include <lcd.h>
#include <delay.h>


void lcd_writeint(const WORD val, BYTE style)
{
    char str[6] = {0, 0, 0, 0, 0, 0};


    if (val < 1) {
        str[0] = 0+48;
        if (style == STYLE_CLK) {
            str[1] = 0+48;
        }
    } else if (val < 10) {
        if (style == STYLE_CLK) {
            str[0] = 0+48;
            str[1] = (unsigned char)( val );
            str[1] += 48;
        } else {
            str[0] = (unsigned char)( val );

            str[0] += 48;
        }
    } else if (val < 100) {
        str[0] = (unsigned char)( val/10 );
        str[1] = (unsigned char)( val-(str[0]*10) );

        str[0] += 48;
        str[1] += 48;
    } else if (val < 1000) {
        str[0] = (unsigned char)( val/100 );
        str[1] = (unsigned char)( (val-(str[0]*100))/10 );
        str[2] = (unsigned char)( (val-((str[0]*100)+(str[1]*10))) );

        str[0] += 48;
        str[1] += 48;
        str[2] += 48;
    } else {
        str[0] = '0';
        str[1] = 0;
        str[2] = 0;
        str[3] = 0;
        str[4] = 0;
        str[5] = 0;
        str[6] = 0;
    }
    lcd_puts(str);
}

void lcd_writefloat(const float num)
{
    BYTE ret;
    unsigned char unum;

    unum = (unsigned char)( num );
    ret = (BYTE) ( (num-unum)*DECIMALAC );

    lcd_writeint((BYTE)num, STYLE_STD);
    lcd_write('.');
    lcd_writeint((BYTE)ret, STYLE_STD);
}


/* write a byte to the LCD in 4 bit mode */

void
lcd_write(unsigned char c)
{
    DelayUs(40);
    RB7 = ((c & 0b10000000) >> 7);
    RB6 = ((c & 0b01000000) >> 6);
    RB5 = ((c & 0b00100000) >> 5);
    RB4 = ((c & 0b00010000) >> 4);
    LCD_STROBE();
    RB7 = ((c & 0b00001000) >> 3);
    RB6 = ((c & 0b00000100) >> 2);
    RB5 = ((c & 0b00000010) >> 1);
    RB4 = ((c & 0b00000001)         );
    LCD_STROBE();
}

/*
 *      Clear and home the LCD
 */

void
lcd_clear(void)
{
    LCD_RS = 0;
    lcd_write(0x1);
    DelayMs(2);
}

/* write a string of chars to the LCD */

void
lcd_puts(const char * s)
{
    LCD_RS = 1;     // write characters
    while(*s)
        lcd_write(*s++);
}

/* write one character to the LCD */

void
lcd_putch(char c)
{
    LCD_RS = 1;     // write characters
    lcd_write( c );
}


/*
 * Go to the specified position
 */

void
lcd_goto(unsigned char pos)
{
    LCD_RS = 0;
    lcd_write(0x80+pos);
}

/* initialise the LCD - put into 4 bit mode */
/*
   void
   lcd_init()
   {
        LCD_RS = 0;
        LCD_EN = 0;

        DelayMs(15);	// wait 15mSec after power applied,
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
        lcd_clear();	// Clear screen
        lcd_write(0x6); // Set entry Mode
   }
 */
