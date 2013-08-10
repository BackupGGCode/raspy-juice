#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

// Command #defines from wiringPi
#define	LCD_CLEAR	0x01
#define	LCD_HOME	0x02
#define	LCD_ENTRY	0x04
#define	LCD_ON_OFF	0x08
#define	LCD_CDSHIFT	0x10
#define	LCD_FUNC	0x20
#define	LCD_CGRAM	0x40
#define	LCD_DGRAM	0x80
#define	LCD_ENTRY_SH	0x01
#define	LCD_ENTRY_ID	0x02
#define	LCD_ON_OFF_B	0x01
#define	LCD_ON_OFF_C	0x02
#define	LCD_ON_OFF_D	0x04
#define	LCD_FUNC_F	0x04
#define	LCD_FUNC_N	0x08
#define	LCD_FUNC_DL	0x10
#define	LCD_CDSHIFT_RL	0x04

// command #defines from Arduino LiquidCrystal.c
#define LCD_CLEARDISPLAY	0x01
#define LCD_RETURNHOME		0x02
#define LCD_ENTRYMODESET	0x04
#define LCD_DISPLAYCONTROL	0x08
#define LCD_CURSORSHIFT		0x10
#define LCD_FUNCTIONSET		0x20
#define LCD_SETCGRAMADDR	0x40
#define LCD_SETDDRAMADDR	0x80
// flags for display entry mode
#define LCD_ENTRYRIGHT		0x00
#define LCD_ENTRYLEFT		0x02
#define LCD_ENTRYSHIFTINCREMENT	0x01
#define LCD_ENTRYSHIFTDECREMENT	0x00
// flags for display on/off control
#define LCD_DISPLAYON		0x04
#define LCD_DISPLAYOFF		0x00
#define LCD_CURSORON		0x02
#define LCD_CURSOROFF		0x00
#define LCD_BLINKON		0x01
#define LCD_BLINKOFF		0x00
// flags for display/cursor shift
#define LCD_DISPLAYMOVE		0x08
#define LCD_CURSORMOVE		0x00
#define LCD_MOVERIGHT		0x04
#define LCD_MOVELEFT		0x00
// flags for function set
#define LCD_8BITMODE		0x10
#define LCD_4BITMODE		0x00
#define LCD_2LINE		0x08
#define LCD_1LINE		0x00
#define LCD_5x10DOTS		0x04
#define LCD_5x8DOTS		0x00

#if 0
/* methods from Arduino's LiquidCrystal.h */
void init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
	  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
void clear();
void home();
void noDisplay();
void display();
void noBlink();
void blink();
void noCursor();
void cursor();
void scrollDisplayLeft();
void scrollDisplayRight();
void leftToRight();
void rightToLeft();
void autoscroll();
void noAutoscroll();
void createChar(uint8_t, uint8_t[]);
void setCursor(uint8_t, uint8_t); 
virtual void write(uint8_t);
void command(uint8_t);
private:
void send(uint8_t, uint8_t);
void write4bits(uint8_t);
void write8bits(uint8_t);
void pulseEnable();
#endif

#if 0
extern void lcd_init(uint8_t dispAttr);
extern void lcd_home(void);
extern void lcd_gotoxy(uint8_t x, uint8_t y);
extern uint8_t lcd_getxy(void);
extern void lcd_putc(char c);
extern void lcd_puts(const char *s);
extern void lcd_command(uint8_t cmd);
extern void lcd_scrollup(void);
#endif

int  daq_lcd_data(int data);
void daq_lcd_strobe(void);
void daq_lcd_regsel(int state);
#define delay(A) usleep(1000UL * (A))

/* LCD device private data */
static uint8_t bits, rows, cols;

static void strobe(void)
{
    daq_lcd_strobe();
}


static void send_lcddata(uint8_t data)
{
    if (bits == 4) {
	daq_lcd_data(data);
	strobe();
	daq_lcd_data(data << 4);
    }
    else {
	daq_lcd_data(data);
    }
    strobe() ;
}


static void send_command(uint8_t command)
{
    daq_lcd_regsel(0);
    send_lcddata(command);
}

static void send_cmd4bit(uint8_t command)
{
    daq_lcd_regsel(0);
    daq_lcd_data(command << 4);
    daq_lcd_strobe();
}


void lcd_home(void)
{
    send_command(LCD_HOME);
}


void lcd_clear(void)
{
    send_command(LCD_CLEAR);
}


void lcd_pos(int row, int col)
{
    static uint8_t rowOff [4] = { 0x00, 0x40, 0x14, 0x54 };
    send_command(col + (LCD_DGRAM | rowOff [row]));
}


void lcd_putchar(uint8_t data)
{
    daq_lcd_regsel(1);
    send_lcddata(data);
}


void lcd_puts(char *string)
{
    while (*string)
	lcd_putchar(*string++);
}


void lcd_printf(char *message, ...)
{
    va_list argp;
    char buffer [1024];

    va_start (argp, message);
    vsnprintf (buffer, 1023, message, argp);
    va_end (argp);
    
    lcd_puts(buffer);
}


int lcd_init(int rows_v, int cols_v, int bits_v)
{
    uint8_t func;
    
    if (! ((bits_v == 4) || (bits_v == 8)))
	return -1 ;
    if ((rows_v < 0) || (rows_v > 20))
	return -1 ;
    if ((cols_v < 0) || (cols_v > 20))
	return -1 ;
    
    bits = 8;
    rows = rows_v;
    cols = cols_v;

    if (bits_v == 4) {
	func = LCD_FUNC | LCD_FUNC_DL;
	send_cmd4bit(func >> 4);
	send_cmd4bit(func >> 4);
	send_cmd4bit(func >> 4);
	func = LCD_FUNC ;
	send_cmd4bit(func >> 4);
	bits = 4 ;
    }
    else {
	func = LCD_FUNC | LCD_FUNC_DL;
	send_command(func     );
	send_command(func     );
	send_command(func     );
    }
    
    if (rows > 1) {
	func |= LCD_FUNC_N ;
	send_command(func);
    }
    
    send_command(LCD_ON_OFF  | LCD_ON_OFF_D);
    send_command(LCD_ENTRY   | LCD_ENTRY_ID);
    send_command(LCD_CDSHIFT | LCD_CDSHIFT_RL);
    send_command(LCD_CLEAR);
    
    return 0;
}
