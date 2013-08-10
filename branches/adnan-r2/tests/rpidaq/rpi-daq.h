#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include "i2c-userspace.h"
#include <linux/spi/spidev.h>

#ifndef RPI_DAQ_H
#define RPI_DAQ_H

int rpi_rev;

extern int fd_i2cbus;
extern int fd_dac1;
extern int fd_dac2;
extern int fd_adc;
extern int i2caddr_dac1;
extern int i2caddr_adc;
extern int i2cbus_open(const char *devbusname);
extern int i2cdev_open(int fd, int i2caddr);
extern int fd_comport;

uint8_t spi_outbuf[16];
uint8_t spi_inbuf[16];
int fd_spi0;
int fd_spi1;
int spi_open(const char *devbusname);
void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx);


#define DAQ_SPISUB_NOP 0
#define DAQ_SPISUB_CFG 1
#define DAQ_SPISUB_LCD 2
#define DAQ_SPISUB_COM 3

#define CFG_MASK_RELAY 0x000f
#define CFG_MASK_LEDS  0x00f0
#define CFG_MASK_I2C_B 0x0100
#define CFG_MASK_AVREN 0x0200

#define MISO_MASK_SW1  0x01
#define MISO_MASK_SW2  0x02
#define MISO_MASK_PH1  0x04
#define MISO_MASK_PH2  0x08

#define UI_SW1		0x01
#define UI_SW2		0x02
#define UI_LED_RED	0x0010
#define UI_LED_AMBER	0x0020
#define UI_LED_GREEN	0x0040


#define RPIRXD_FROM_DUT_HDR_RXD	0x01
#define RPIRXD_FROM_CONSOLE_TX	0x02
#define RPIRXD_FROM_AVR232_TX	0x03
#define RPIRXD_FROM_AVR485_TX	0x04

#define RPITXD_TO_DUT_HDR_TXD	0x10
#define RPITXD_TO_CONSOLE_RX	0x20
#define RPITXD_TO_AVR232_RX	0x30
#define RPITXD_TO_AVR485_RX	0x40


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
void daq_set_comms_matrix(int setting);

int  lcd_init(int rows, int cols, int bits);
void lcd_clear(void);
void lcd_home(void);
void lcd_pos(int row, int col);
void lcd_putc(uint8_t data);
void lcd_puts(char *string);
void lcd_printf(char *message, ...);

int  rpi_comport_open(char *devpathname);
int  rpi_comport_setbaud(int baudrate);
void rpi_comport_close(void);


#endif /* RPI_DAQ_H */