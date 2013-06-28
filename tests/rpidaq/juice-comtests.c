#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * From TLDP Serial Programming Example
 * http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
 */
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */

#include "rpi-daq.h"
#include "lib-juice.h"

#define CT_BUFSIZE 255
char avr_inbuf[CT_BUFSIZE+1];
char avr_outbuf[CT_BUFSIZE+1];
char rpi_inbuf[CT_BUFSIZE+1];
char rpi_outbuf[CT_BUFSIZE+1];

#define msleep(MS)	usleep(1000 * (MS))


/*
 * Reads RPi console and AVR232 buffers through I2C, and compares results
 */
static int avr232_readtest(int retries, int timeout)
{
    int r, avr_n, rpi_n;
    
    memset(avr_inbuf, 0, CT_BUFSIZE);
    memset(rpi_inbuf, 0, CT_BUFSIZE);
    avr_n = rpi_n = 0;
    retries = 10;
    
    while (retries--) {
	while (rj_readstat() & RXA232) {
	    r = rj232_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
	    avr_n += r;
	}
	
	r = read(fd_comport, &rpi_inbuf[rpi_n], CT_BUFSIZE - rpi_n - 1);
	if (r > 0) {
	    rpi_n += r;
	}
	msleep(10);
    }
    rpi_inbuf[rpi_n] = 0;
    
    printf("avr_inbuf: %02d bytes: %s\n", avr_n, avr_inbuf);
    printf("rpi_inbuf: %02d bytes: %s\n", rpi_n, rpi_inbuf);
    
    /* Do comparison tests with outbufs */
    return 0;
}


static int test_avr232_egress(int run)
{
    int r;
    
    printf("%s:\n", __func__);
    daq_set_comms_matrix(RPIRXD_FROM_AVR232_TX);
    msleep(10);
    
    sprintf(avr_outbuf, "RPi I2C >>>>> AVR RS232 TX >>>>> RPi RxD");
    r =	strlen(avr_outbuf);
    rj232_send((unsigned char *)avr_outbuf, r);
    printf("rj232_send: %d bytes sent.\n", r);
    
    return avr232_readtest(50, 1);
}

static int test_avr232_ingress(int run)
{
    int r;

    printf("%s:\n", __func__);
    daq_set_comms_matrix(RPITXD_TO_AVR232_RX);
    msleep(10);

    sprintf(rpi_outbuf, "RPi I2C <<<<<< AVR RS232 RX <<<<<< RPi Txd");
    r = write(fd_comport, rpi_outbuf, strlen(rpi_outbuf));
    printf("write(ttyAMA0): %d bytes sent.\n", r);
    msleep(250);

    return avr232_readtest(50, 1);
}


/*
 * Tests the auxiliary RS232 interface of the AVR microcontroller on Juice
 * The interface is a HALF Duplex, so it cannot simultaneously send/receive
 * data. A wait in between the transactions is neccessary.
 */
int test_avr232_comms(int randrun)
{
    if (rpi_comport_setbaud(B9600)) {
	printf("%s: rpi_comport_setbaud failed.\n", __func__);
	return -1;
    }
    if (rj232_setbaud(9600)) {
	printf("%s: rj232_setbaud failed.\n", __func__);
	return -2;
    }

    /* clean out rj232 buffer */
    while(rj_readstat() & RXA232) {
	rj232_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
    }
    
    printf("AVR RS232 Egress Test\n");
    if (test_avr232_egress(1)) {
	printf("AVR RS232 Egress: failed\n");
	return -5;
    }
    
    printf("AVR RS232 Ingress Test\n");
    if (test_avr232_ingress(1)) {
	printf("AVR RS232 Ingress: failed\n");
	return -6;
    }
    
    printf("AVR RS232 Test: Passed\n\n");
    return 0;
}


static int test_console_echo(int out_in)
{
    int retries, r, rpi_out, rpi_in;
    
    if (out_in) {
	sprintf(rpi_outbuf, "RPi Header TxD >>>>> Console RS232 out");
    }
    else {
	sprintf(rpi_outbuf, "RPi Header Rxd <<<<< Console RS232 in");
    }
    rpi_out = write(fd_comport, rpi_outbuf, strlen(rpi_outbuf));
    
    rpi_in = 0;
    retries = 5;
    memset(rpi_inbuf, 0, CT_BUFSIZE);

    while (retries--) {
	r = read(fd_comport, &rpi_inbuf[rpi_in], CT_BUFSIZE - rpi_in - 1);
	if (r > 0) {
	    rpi_in += r;
	}
	msleep(10);
    }
    rpi_inbuf[rpi_in] = 0;

    printf("rpi_outbuf: %02d bytes: %s\n", rpi_out, rpi_outbuf);
    printf("rpi_inbuf : %02d bytes: %s\n", rpi_in, rpi_inbuf);
    
    /* Do comparisons */
    if (rpi_out != rpi_in)
	return -1;
    return memcmp(rpi_outbuf, rpi_inbuf, rpi_out);
}


/*
 * Tests the Raspberry Pi console connections from GPIO header to RS232-level
 * header pin, and from the RS232-level Rx pin to the GPIO header.
 */ 
int test_con232_comms(int randrun)
{
    
    rpi_comport_setbaud(B115200);

    /* Console RS232 Egress */
    printf("Console RS232 Egress Test\n");
    daq_set_comms_matrix(RPITXD_TO_DUT_HDR_TXD | RPIRXD_FROM_CONSOLE_TX);
    if (test_console_echo(0)) {
	printf("Console RS232 egress:  failed.\n");
	return -2;
    }
	
    /* Console RS232 Ingress */
    printf("Console RS232 Ingress Test\n");
    daq_set_comms_matrix(RPITXD_TO_CONSOLE_RX | RPIRXD_FROM_DUT_HDR_RXD);
    if (test_console_echo(1)) {
	printf("Console RS232 ingress: failed.\n");
	return -3;
    }

    printf("Console RS232 Test: passed\n\n");
    return 0;
}

/*
 * Tests the auxiliary RS232 interface of the AVR microcontroller on Juice
 * The interface is a HALF Duplex, so it cannot simultaneously send/receive
 * data. A wait in between the transactions is neccessary.
 */
int test_avr485_comms(int randrun)
{
    int r, retries, out_n, avr_in, rpi_in;
    
    rpi_comport_setbaud(B115200);
    
    if (rj485_setbaud(115200)) {
	printf("%s: rj485_setbaud failed.\n", __func__);
	return -2;
    }

    /* flush rj485 buffer */
    while(rj_readstat() & RXA485) {
	rj485_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
    }

/*
#define RPIRXD_FROM_DUT_HDR_RXD	0x01
#define RPIRXD_FROM_CONSOLE_TX	0x02
#define RPIRXD_FROM_AVR232_TX	0x03
#define RPIRXD_FROM_AVR485_TX	0x04

#define RPITXD_TO_DUT_HDR_TXD	0x10
#define RPITXD_TO_CONSOLE_RX	0x20
#define RPITXD_TO_AVR232_RX	0x30
#define RPITXD_TO_AVR485_RX	0x40
*/
    printf("AVR RS485 Egress Test\n");
    daq_set_comms_matrix(RPIRXD_FROM_AVR485_TX);
    sprintf(avr_outbuf, "RPi >> I2C >> DUT RS485 >> xAA55aa55x >> RPi RS485 RxD");
    out_n = strlen(avr_outbuf);
    rj485_send((unsigned char *) avr_outbuf, out_n);
    
    memset(avr_inbuf, 0, CT_BUFSIZE);
    memset(rpi_inbuf, 0, CT_BUFSIZE);
    avr_in = rpi_in = 0;
    retries = 10;
    
    while (retries--) {
	while (rj_readstat() & RXA485) {
	    r = rj232_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
	    avr_in += r;
	}
	
	r = read(fd_comport, &rpi_inbuf[rpi_in], CT_BUFSIZE - rpi_in - 1);
	if (r > 0) {
	    rpi_in += r;
	}
	msleep(10);
    }
    rpi_inbuf[rpi_in] = 0;
    
    printf("avr_outbuf: %02d bytes: %s\n", out_n,  avr_outbuf);
    printf("avr_inbuf:  %02d bytes: %s\n", avr_in, avr_inbuf);
    printf("rpi_inbuf:  %02d bytes: %s\n", rpi_in, rpi_inbuf);
    
    /* Egress comparisons */

    
    
    
    

    /* AVR RS485 Ingress TEST */
    
    printf("AVR RS485 Ingress Test\n");
    daq_set_comms_matrix(RPITXD_TO_AVR485_RX);
    sprintf(rpi_outbuf, "RPi >> RS485 TxD >> xAA55aa55x >> DUT RS485 >> I2C >> RPi");
    out_n = strlen(rpi_outbuf);
    
    /* flush rj485 buffer */
    while(rj_readstat() & RXA485) {
	rj485_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
    }

    write(fd_comport, rpi_outbuf, out_n);


    memset(avr_inbuf, 0, CT_BUFSIZE);
    memset(rpi_inbuf, 0, CT_BUFSIZE);
    avr_in = rpi_in = 0;
    retries = 10;
    
    while (retries--) {
	while (rj_readstat() & RXA485) {
	    r = rj485_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
	    avr_in += r;
	}
	
	r = read(fd_comport, &rpi_inbuf[rpi_in], CT_BUFSIZE - rpi_in - 1);
	if (r > 0) {
	    rpi_in += r;
	}
	msleep(10);
    }
    rpi_inbuf[rpi_in] = 0;
    
    printf("rpi_outbuf: %02d bytes: %s\n", out_n,  rpi_outbuf);
    printf("avr_inbuf:  %02d bytes: %s\n", avr_in, avr_inbuf);
    printf("rpi_inbuf:  %02d bytes: %s\n", rpi_in, rpi_inbuf);
    
    daq_set_comms_matrix(0);
   
    
    
    return 0;
}



int test_juice_comms(int randrun)
{
    if (test_con232_comms(1))
	return -1;
    if (test_avr232_comms(1))
	return -2;
    if (test_avr485_comms(1))
	return -3;
    return 0;
}