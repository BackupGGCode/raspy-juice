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

int fd_i2cbus = 0;
int fd_dac1;
int fd_dac2;
int fd_adc;
int i2caddr_dac1 = 0x60;
int i2caddr_adc = 0x69;
int i2cbus_open(const char *devbusname);
int i2cdev_open(int fd, int i2caddr);

uint8_t spi_outbuf[16];
uint8_t spi_inbuf[16];
int fd_spi0;
int fd_spi1;
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
int spi_open(const char *devbusname);
void spi_xfer(int fd, int len, uint8_t *tx, uint8_t *rx);

void daq_set_relay(int relay, int onoff);
void daq_set_relays(int relay_set);
void daq_set_led(int led, int onoff);
void daq_set_leds(int led_set);
int  daq_lcd_data(int data);
void daq_lcd_strobe(void);
void daq_lcd_regsel(int state);

int lcd_main (void);

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
    
    return 0;
   
#if 0
    //err = i2cdev_open(fd_i2cbus, i2caddr_adc);
    err = ioctl(fd_i2cbus, I2C_SLAVE, i2caddr_adc);
    printf("i2cdev_open: err = %d\n", err);
    fd_adc = err;
    numbytes = read(fd_i2cbus, i2c_buffer, 4);
    printf("read: numbytes read = %d, 0x%02x 0x%02x 0x%02x 0x%02x\n",
	   numbytes, 
	   i2c_buffer[0], i2c_buffer[1], i2c_buffer[2], i2c_buffer[3]);
#endif
       
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
    
#if 0    
    uint8_t tx[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
	0xF0, 0x8D,
    };
    uint8_t rx[ARRAY_SIZE(tx)] = {0, };
#endif
    
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
    
    for (ret = 0; ret < len; ret++) {
	if (!(ret % 6))
	    puts("");
	printf("%.2X ", rx[ret]);
	}
    puts("");
}



void daq_set_relay(int relay, int onoff)
{
    int mask = 1 << relay;
    spi_outbuf[3] &= ~mask;
    if (onoff) {
	spi_outbuf[3] |= mask;
    }
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
}

void daq_set_relays(int relay_set)
{
    spi_outbuf[3] &=  ~0x0f;
    spi_outbuf[3] |= (relay_set & 0x0f);
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
}

void daq_set_led(int led, int onoff)
{
    int mask = 1 << (led + 4);
    spi_outbuf[3] &= ~mask;
    if (onoff) {
	spi_outbuf[3] |= mask;
    }
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
}

void daq_set_leds(int led_set)
{
    spi_outbuf[3] &= ~0xf0;
    spi_outbuf[3] |= ((led_set & 0x0f) << 4);
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
}


int  daq_lcd_data(int data)
{
    spi_outbuf[1] = data;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
}

void daq_lcd_regsel(int state)
{
    int mask = 0x01;
    spi_outbuf[0] &= ~mask;
    if (state) {
	spi_outbuf[0] |= mask;
    }
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    printf("lcd reg selected = %d\n", state);
}

void daq_lcd_strobe(void)
{
    int mask = 0x04;
    /* lcd strobe E# is inverted in CPLD implementation */
    /* assert */
    spi_outbuf[0] |= mask;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    usleep(50);
    /* un-assert */
    spi_outbuf[0] &= ~mask;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    usleep(50);
    printf("lcd strobed\n");
}





int main(int argc, char *argv[])
{
    int err, numbytes;
    unsigned char i2c_buffer[16];
    
    printf("Hello, world!\n\n");
    
    rpi_rev = 1;
    err = i2cbus_open("/dev/i2c-0");

    if (err < 0) {
	rpi_rev = 2;
	err = i2cbus_open("/dev/i2c-1");
    }
    
    if (err < 0) {
	printf("i2cbus_open: /dev/i2c-x unsuccessful, rpidaq not found.\n");
	exit(1);
    }
    
    printf("i2cbus_open: successful, rpi_rev = %d\n", rpi_rev);

    fd_spi0 = spi_open("/dev/spidev0.0");
    if (fd_spi0 < 0) {
	printf("spi_open: /dev/spidev0.0 unsuccessful.\n");
	exit(1);
    }

    fd_spi1 = spi_open("/dev/spidev0.1");
    if (fd_spi1 < 0) {
	printf("spi_open: /dev/spidev0.1 unsuccessful.\n");
	exit(1);
    }
    
    spi_outbuf[0] = 0x00;
    spi_outbuf[1] = 0x00;
    spi_outbuf[2] = 0x00;
    spi_outbuf[3] = 0x00;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    sleep(1);

    daq_set_relay(0, 1);
    sleep(2);

#if 0    
    daq_set_relay(1, 1);
    sleep(1);
    
    daq_set_relay(2, 1);
    sleep(1);
    
    daq_set_relay(3, 1);
    sleep(1);
    
    daq_set_relay(0, 0);
    sleep(1);
    
    daq_set_relay(1, 0);
    sleep(1);
    
    daq_set_relay(2, 0);
    sleep(1);
    
    daq_set_relay(3, 0);
    sleep(1);
    
    daq_set_led(0, 1);
    sleep(1);
    
    daq_set_led(1, 1);
    sleep(1);
    
    daq_set_led(2, 1);
    sleep(1);
    
    daq_set_led(0, 0);
    sleep(1);
    
    daq_set_led(1, 0);
    sleep(1);
    
    daq_set_led(2, 0);
    sleep(1);
#endif
    
#if 1
    daq_lcd_data(0x00);
    daq_lcd_regsel(0);
    
    printf("Going into lcd_main()\n");
    lcd_main ();
    printf("Got out of lcd_main()\n");
#endif
    
#if 0
    spi_outbuf[0] = 0xff;
    spi_outbuf[1] = 0xff;
    spi_outbuf[2] = 0xff;
    spi_outbuf[3] = 0x20;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    sleep(1);
    
    spi_outbuf[0] = 0xff;
    spi_outbuf[1] = 0xff;
    spi_outbuf[2] = 0xff;
    spi_outbuf[3] = 0x21;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    sleep(1);
    
    spi_outbuf[0] = 0xff;
    spi_outbuf[1] = 0xff;
    spi_outbuf[2] = 0xff;
    spi_outbuf[3] = 0x23;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    sleep(1);

    spi_outbuf[0] = 0xff;
    spi_outbuf[1] = 0xff;
    spi_outbuf[2] = 0xff;
    spi_outbuf[3] = 0x27;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    sleep(1);
    
    spi_outbuf[0] = 0xff;
    spi_outbuf[1] = 0xff;
    spi_outbuf[2] = 0xff;
    spi_outbuf[3] = 0x2f;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
    sleep(1);
#endif
//    spi_outbuf[0] = 0x0;
//    spi_outbuf[1] = 0x0;
    spi_outbuf[2] = 0x0;
    spi_outbuf[3] = 0x10;
    spi_xfer(fd_spi0, 4, spi_outbuf, spi_inbuf);
   
}


