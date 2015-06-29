#include <Standarddefines.h>

#define LCD_RS RA4 // Register Select
#define LCD_EN RA3 // Enable

#define LCD_DATA        PORTB // Runs In 4 bit mode, uses bits 4,5,6,7

#define LCD_STROBE()    ((LCD_EN = 1),(LCD_EN=0))

#define STYLE_STD 0
#define STYLE_CLK 1

#define DECIMALAC 100

void lcd_writeint(const WORD val, BYTE style);
void lcd_writefloat(const float num);

/* write a byte to the LCD in 4 bit mode */

extern void lcd_write(unsigned char);

/* Clear and home the LCD */

extern void lcd_clear(void);

/* write a string of characters to the LCD */

extern void lcd_puts(const char * s);

/* Go to the specified position */

extern void lcd_goto(unsigned char pos);

/* intialize the LCD - call before anything else */

//extern void lcd_init(void);

extern void lcd_putch(char);

/*	Set the cursor position */

#define lcd_cursor(x)   lcd_write(((x)&0x7F)|0x80)
