#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include "i2c-userspace.h"
#include <linux/spi/spidev.h>

int rpi_rev;

int fd_i2cbus;
int fd_dac1;
int fd_dac2;
int fd_adc;
int i2caddr_dac1;
int i2caddr_adc;
int i2cbus_open(const char *devbusname);
int i2cdev_open(int fd, int i2caddr);

uint8_t spi_outbuf[16];
uint8_t spi_inbuf[16];
int fd_spi0;
int fd_spi1;
int spi_open(const char *devbusname);
void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx);

#define DAQ_SPISUB_CFG 1
#define DAQ_SPISUB_LCD 2
#define DAQ_SPISUB_COM 3

#define CFG_MASK_RELAY 0x000f
#define CFG_MASK_LEDS  0x00f0
#define CFG_MASK_I2C_B 0x0100
#define CFG_MASK_AVREN 0x0200

int rpi_daq_init(void);
int rpi_daq_close(void);

int i2cbus_open(const char *devbusname);    
int spi_open(const char *devbusname);
void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx);

int  daq_xfer(int subsystem, int dout);
int  daq_lcd_data(int data);
void daq_lcd_regsel(int addr);
void daq_lcd_strobe(void);
void daq_set_relay(int relay, int onoff);
void daq_set_led(int led, int onoff);
void daq_set_buffered_i2c(int onoff);
void daq_set_buffered_avr(int onoff);
void daq_set_com_matrix(int setting);

int  lcd_init(int rows, int cols, int bits);
void lcd_clear(void);
void lcd_home(void);
void lcd_pos(int row, int col);
void lcd_putc(uint8_t data);
void lcd_puts(char *string);
void lcd_printf(char *message, ...);
