#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include "i2c-userspace.h"
#include <linux/spi/spidev.h>


#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

int rpi_rev;

int fd_i2cbus;
int fd_dac1;	/* unused */
int fd_dac2;	/* unused */
int fd_adc;	/* unused */
int i2caddr_dac1 = 0x60;
int i2caddr_dac2 = 0x61;	/* unused */
int i2caddr_adc = 0x69;

uint8_t spi_outbuf[16];
uint8_t spi_inbuf[16];
int fd_spi0;
int fd_spi1;
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

#define DAQ_SPISUB_CFG 1
#define DAQ_SPISUB_LCD 2
#define DAQ_SPISUB_COM 3

static int cfg_data;
static int lcd_data;

#define CFG_MASK_RELAY 0x000f
#define CFG_MASK_LEDS  0x00f0
#define CFG_MASK_I2C_B 0x0100
#define CFG_MASK_AVREN 0x0200

#define BAUDRATE	B9600
#define COMPORT_DEVICE	"/dev/ttyAMA0"
#define _POSIX_SOURCE 	1 /* POSIX compliant source */

#define msleep(MS) 	usleep(1000 * (MS))

int fd_comport = 0;
static struct termios oldtio;


int i2cbus_open(const char *devbusname)
{
    int rval;
    unsigned char i2c_buffer[16];

    /* test bus */
    fd_i2cbus = open(devbusname, O_RDWR);
    if (fd_i2cbus < 0)
	return -1;

    /* setup ADC device as slave*/
    fd_adc = ioctl(fd_i2cbus, I2C_SLAVE, i2caddr_adc);
    if (fd_adc < 0)
       return -2;
    
    /* read ADC device */
    rval = read(fd_i2cbus, i2c_buffer, 4);
    if (rval < 0)
	return -3;

    return fd_i2cbus;
}
    
int spi_open(const char *devbusname)
{
    int ret, fd;
    
    fd = open(devbusname, O_RDWR);
    if (fd < 0) {
	printf("spi_open: can't open device\n");
	return -1;
    }
    
    /* spi mode */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
	printf("can't set spi mode\n");
	return -2;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1) {
	printf("spi_open: can't get spi mode\n");
	return -3;
    }
    
    /* bits per word */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
	printf("spi_open: can't set bits per word\n");
	return -4;
    }
    
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) {
	printf("spi_open: can't get bits per word\n");
	return -5;
    }
    
    /* max speed hz */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
	printf("spi_open: can't set max speed hz\n");
	return -6;
    }
    
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
	printf("spi_open: can't get max speed hz");
	return -7;
    }
    
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    
    return fd;
}

void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx)
{
    int ret;
    
    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = len,
	.delay_usecs = delay,
	.speed_hz = speed,
	.bits_per_word = bits,
    };
    
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
	printf("spi_xfer: can't send spi message");
#if 0
    for (ret = 0; ret < len; ret++) {
	if (!(ret % 6))
	    puts("");
	printf("%.2X ", rx[ret]);
	}
    puts("");
#endif
}

int daq_xfer(int subsystem, int dout)
{
    spi_outbuf[0] = subsystem >> 8;
    spi_outbuf[1] = subsystem;
    spi_xfer(fd_spi1, 2, spi_outbuf, spi_inbuf);
    
    spi_outbuf[0] = dout >> 8;
    spi_outbuf[1] = dout;
    spi_xfer(fd_spi0, 2, spi_outbuf, spi_inbuf);
    
#if 0    
    printf("daq_xfer: spi_inbuf[1, 0] = %02X %02X\n", spi_inbuf[1], spi_inbuf[0]);
#endif
    
    return (spi_inbuf[1] << 8) | (spi_inbuf[0] & 0xff);
}

int  daq_lcd_data(int data)
{
    lcd_data = lcd_data & 0xff00;
    lcd_data = lcd_data | (data & 0xff);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    usleep(50);
    return 0;
}

void daq_lcd_regsel(int addr)
{
    lcd_data = lcd_data & (~0x0100);
    lcd_data = lcd_data | (addr ? 0x0100 : 0);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    usleep(50);
}

void daq_lcd_strobe(void)
{
    /* lcd strobe E# is inverted in CPLD implementation */
    /* assert */
    lcd_data = lcd_data & (~0x0400);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    usleep(50);
    /* unassert */
    lcd_data = lcd_data | (0x0400);
    daq_xfer(DAQ_SPISUB_LCD, lcd_data);
    usleep(50);
}

void daq_set_relay(int relay, int onoff)
{
    int mask;
    if ((relay < 0) || (relay > 3))
	return;
    mask = 1 << relay;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);    
}

void daq_set_led(int led, int onoff)
{
    int mask;
    if ((led < 0) || (led > 3))
	return;
    mask = 1 << (led + 4);
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);    
}

void daq_set_buffered_i2c(int onoff)
{
    int mask;
    mask = CFG_MASK_I2C_B;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);
}

void daq_set_buffered_avr(int onoff)
{
    int mask;
    mask = CFG_MASK_AVREN;
    if (onoff) {
	cfg_data |= mask;
    }
    else {
	cfg_data &= ~mask;
    }
    daq_xfer(DAQ_SPISUB_CFG, cfg_data);
}

void daq_set_comms_matrix(int setting)
{
    daq_xfer(DAQ_SPISUB_COM, setting);
}

int rpi_daq_init(void)
{
    int i;
    
    rpi_rev = 1;
    
    /* Test of SPI dev and connectivity to DAQ ADC chip */
    fd_i2cbus = i2cbus_open("/dev/i2c-0");
    
    if (fd_i2cbus < 0) {
	rpi_rev = 2;
	fd_i2cbus = i2cbus_open("/dev/i2c-1");
    }
    
    if (fd_i2cbus < 0) {
	printf("i2cbus_open: /dev/i2c-x unsuccessful, rpidaq not found.\n");
	return -1;
    }
    printf("i2cbus_open: successful, fd_i2cbus = %d, rpi_rev = %d\n",
	   fd_i2cbus,
	   rpi_rev);

    fd_spi0 = spi_open("/dev/spidev0.0");
    if (fd_spi0 < 0) {
	printf("spi_open: /dev/spidev0.0 unsuccessful.\n");
	return -1;
    }
    printf("spi_open: successful, fd_spi0 = %d\n", fd_spi0);

    fd_spi1 = spi_open("/dev/spidev0.1");
    if (fd_spi1 < 0) {
	printf("spi_open: /dev/spidev0.1 unsuccessful.\n");
	return -1;
    }
    printf("spi_open: successful, fd_spi1 = %d\n", fd_spi1);

    cfg_data = 0;
    lcd_data = 0;
    daq_lcd_data(0x00);
    daq_lcd_regsel(0);
    daq_set_buffered_avr(0);
    daq_set_buffered_i2c(0);
    daq_set_comms_matrix(0x00);

    for (i = 0; i < 4; i++) {
	daq_set_led(i, 0);
	daq_set_relay(i, 0);
    }
    return 0;
}

void rpi_daq_close(void)
{
    close(fd_i2cbus);
    close(fd_spi0);
    close(fd_spi1);
}


int rpi_comport_open(char *devpathname)
{
    struct termios newtio;
    
    /* If already opened, just return the fd */
    if (fd_comport > 0)
	return fd_comport;
    /* open the device to be non-blocking (read will return immediately) */
    fd_comport = open(devpathname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_comport < 0) {
	perror(devpathname);
	return -1;
    }
    /* save current port settings */
    tcgetattr(fd_comport, &oldtio);
    /* set new port settings for raw & non-canonical input processing */
    cfmakeraw(&newtio);
    cfsetspeed(&newtio, B9600);
    /* minimum one character, or 1 character timeout */
    newtio.c_cc[VMIN] = 0;
    newtio.c_cc[VTIME] = 0;
    tcflush(fd_comport, TCIOFLUSH);
    tcsetattr(fd_comport, TCSANOW, &newtio);
    return 0;
}

int rpi_comport_setbaud(int baudrate)
{
    struct termios tmptio;

    if (fd_comport <= 0) {
	fprintf(stderr, "Error in coding: RPi comport wasn't opened.\n");
	exit(-1);
    }
    /* get current port settings */
    tcgetattr(fd_comport, &tmptio);
    /* set new baud rate and preserve everything else */
    cfsetspeed(&tmptio, baudrate);
    tcdrain(fd_comport);
    tcflush(fd_comport, TCIOFLUSH);
    //msleep(500);
    tcsetattr(fd_comport, TCSANOW, &tmptio);
    msleep(500);
    return 0;
}

void rpi_comport_close(void)
{
    if (fd_comport > 0) {
	tcflush(fd_comport, TCIFLUSH);
	/* restore old port settings */
	tcsetattr(fd_comport, TCSANOW, &oldtio);
	close(fd_comport);
	fd_comport = 0;
    }
}
