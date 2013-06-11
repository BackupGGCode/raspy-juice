#ifndef __JUICE_DEV_H__
#define __JUICE_DEV_H__

//#include "../firmware/juice.h"

/* Addresses of I2C slaves on Juice PCBA */
#define AVRSLAVE_ADDR	(unsigned char) 0x48
#define PCF8523_ADDR	(unsigned char) 0x68

/* Addresses of Juice emulated AVR I2C registers */
#define GSTAT	0x00
#define RXA232	0x01	/* RS232 data available */
#define RXA485	0x04	/* RS485 data available */
#define EEBUSY	0x20	/* EEPROM write in progress */
#define ADCBUSY	0x40	/* ADC conversion in progress */

/* AVR PC0..PC3 servo pulse width setting registers */
#define SERVO_0	0x01
#define SERVO_1	0x02
#define SERVO_2	0x03
#define SERVO_3	0x04
#define	SERVO_4	0x05
#define SERVO_5	0x06
#define SERVO_6	0x07
#define SERVO_7	0x08

/* AVR RS232 registers */
#define RS232D	0x10
#define BPS232 0x15

/* AVR RS485 registers */
#define RS485D	0x20
#define BPS485	0x25

/* AVR ADC set and readback registers */
#define ADCMUX	0x40
#define ADCDAT	0x41

/* AVR ADC EEPROM registers */
#define EEADDR	0x50
#define EEDATA	0x51

/* AVR Reboot register and constant to write to reboot*/
#define REBOOT	0xb0
#define BOOTVAL	0x0d
#define RJ_VERSION	0xb1




#define BUFSIZE 64
char version_str[BUFSIZE];
unsigned char rs232_inbuf[BUFSIZE];
unsigned char rs485_inbuf[BUFSIZE];

int rj_readbyte(int subreg);
int rj_writebyte(int subreg, int data);
int rj_readword(int subreg);
int rj_writeword(int subreg, int data);

int rj_open(const char *devbusname, int i2caddr);
char *rj_getversion(void);
int rj_setservo(int chan, int usec);
int rj_readstat(void);
int rj_readadc(unsigned char mux);

int rj232_setbaud(int speed);
int rj232_getc(void);
int rj232_read(unsigned char *buf, int maxlen);
int rj232_send(unsigned char *buf, int len);

int rj485_setbaud(int speed);
int rj485_getc(void);
int rj485_read(unsigned char *buf, int maxlen);
int rj485_send(unsigned char *buf, int len);

#endif /* __JUICE_DEV_H__ */
