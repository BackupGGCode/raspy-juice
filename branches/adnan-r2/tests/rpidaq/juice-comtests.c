#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "rpi-daq.h"
#include "lib-juice.h"

#define msleep(MS)	usleep(1000 * (MS))

#define READ_RETRIES 10
#define TEST_RETRIES 10


#define CT_BUFSIZE 255
static char outbuf[CT_BUFSIZE + 1];
static char rpi_inbuf[CT_BUFSIZE + 1];
static char avr232_inbuf[CT_BUFSIZE + 1];
static char avr485_inbuf[CT_BUFSIZE + 1];
static int  out_n, rpi_in, avr232_in, avr485_in;

static void rj_flush(void)
{
    int retries;
    
    /* flush out rj232 and rj485 buffers */
    retries = 10;
    while (retries-- && (rj_readstat() & RXA232)) {
	rj232_read((unsigned char *)avr232_inbuf, CT_BUFSIZE);
    }
    retries = 10;
    while (retries-- && (rj_readstat() & RXA485)) {
	rj485_read((unsigned char *)avr485_inbuf, CT_BUFSIZE);
    }
    memset(avr232_inbuf, 0, CT_BUFSIZE);
    memset(avr485_inbuf, 0, CT_BUFSIZE);
}

static void printbufs(void)
{
    printf("      outbuf: %02d bytes: %s\n", out_n,    outbuf);
    printf("   rpi_inbuf: %02d bytes: %s\n", rpi_in,    rpi_inbuf);
    printf("avr232_inbuf: %02d bytes: %s\n", avr232_in, avr232_inbuf);
    printf("avr485_inbuf: %02d bytes: %s\n", avr485_in, avr485_inbuf);
}

/*
 * Reads all RPi comport, AVR232 and AVR485 buffers.
 * Returns number of bytes in each inbuf by integer refs.
 * Returns number of retries to complete.
 */
static int read_inbufs(void)
{
    int retries, stat, r;
    
    retries = READ_RETRIES;
    rpi_in = avr232_in = avr485_in = 0;
    
    memset(rpi_inbuf, 0, CT_BUFSIZE);
    memset(avr232_inbuf, 0, CT_BUFSIZE);
    memset(avr485_inbuf, 0, CT_BUFSIZE);

    while (retries--) {
	stat = rj_readstat();
	
	if (stat & RXA232) {
	    r = rj232_read((unsigned char *)avr232_inbuf + avr232_in, 
			   CT_BUFSIZE - avr232_in -1);
	    avr232_in += r;
	}
	if (stat & RXA485) {
	    r = rj485_read((unsigned char *)avr485_inbuf + avr485_in,
			   CT_BUFSIZE - avr485_in - 1);
	    avr485_in += r;
	}
	r = read(fd_comport, rpi_inbuf + rpi_in, CT_BUFSIZE - rpi_in - 1);
	if (r > 0) {
	    rpi_in += r;
	}
	msleep(10);
    }
    //printbufs();

    return READ_RETRIES - retries;    
}

int test_con232_egress(int randrun)
{
    daq_set_comms_matrix(RPITXD_TO_DUT_HDR_TXD | RPIRXD_FROM_CONSOLE_TX);
    
    rpi_comport_setbaud(B115200);
    
    //printf("Console RS232 Egress Test\n");
    sprintf(outbuf, "RPi Header TxD >>sayHello>>> Console RS232 out");
    out_n = strlen(outbuf);
    
    write(fd_comport, outbuf, out_n);
    
    read_inbufs();

    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, outbuf, out_n))
	return -3;
    return 0;
}

int test_con232_ingress(int randrun)
{
    daq_set_comms_matrix(RPITXD_TO_CONSOLE_RX | RPIRXD_FROM_DUT_HDR_RXD);
    
    rpi_comport_setbaud(B57600);

    //printf("Console RS232 Ingress Test\n");
    sprintf(outbuf, "RPi Header Rxd <<someReply<<< Console RS232 in");
    out_n = strlen(outbuf);
    
    write(fd_comport, outbuf, out_n);
    
    read_inbufs();
            
    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, outbuf, out_n))
	return -3;
    return 0;
}

/*
 * Tests the auxiliary RS232 interface of the AVR microcontroller on Juice
 * The interface is a HALF Duplex, so it cannot simultaneously send/receive
 * data. A wait in between the transactions is neccessary.
 */
int test_avr232_egress(int randrun)
{
    daq_set_comms_matrix(RPIRXD_FROM_AVR232_TX);
    
    rj232_setbaud(9600);
    rpi_comport_setbaud(B9600);

    //printf("AVR RS232 Egress Test\n");
    sprintf(outbuf, "RPi I2C >>>>> AVR RS232 TX >>>>> RPi RxD");
    out_n = strlen(outbuf);

    rj_flush();
    
    rj232_send((unsigned char *)outbuf, out_n);
    
    read_inbufs();
    
    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, outbuf, out_n))
	return -3;
    return 0;
}

int test_avr232_ingress(int randrun)
{
    daq_set_comms_matrix(RPITXD_TO_AVR232_RX);

    rj232_setbaud(9600);
    rpi_comport_setbaud(B9600);
    
    //printf("AVR RS232 Ingress Test\n");
    sprintf(outbuf, "RPi I2C <<<AVR RS232 RX <<< RPi Txd");
    out_n = strlen(outbuf);
    
    rj_flush();

    write(fd_comport, outbuf, out_n);
    
    read_inbufs();

    if (avr232_in != out_n)
	return -2;
    if (memcmp(avr232_inbuf, outbuf, out_n))
	return -3;
    return 0;
}

int test_avr485_egress(int randrun)
{
    daq_set_comms_matrix(RPIRXD_FROM_AVR485_TX);

    rj485_setbaud(115200);
    rpi_comport_setbaud(B115200);
    
    //printf("AVR RS485 Egress Test\n");
    sprintf(outbuf, "RPi >> I2C >> DUT RS485 >> xAA55aa55x >> RPi RS485 RxD");
    out_n = strlen(outbuf);
    
    rj_flush();

    rj485_send((unsigned char *) outbuf, out_n);
    
    read_inbufs();

    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, outbuf, out_n))
	return -3;
    return 0;
}

int test_avr485_ingress(int randrun)
{
    daq_set_comms_matrix(RPITXD_TO_AVR485_RX);

    rj485_setbaud(115200);
    rpi_comport_setbaud(B115200);
    
    //printf("AVR RS485 Ingress Test\n");
    sprintf(outbuf, "RPi >> RS485 TxD >> xAA55aa55x >> DUT RS485 >> I2C >> RPi");
    out_n = strlen(outbuf);
    
    rj_flush();

    write(fd_comport, outbuf, out_n);

    read_inbufs();
    
    daq_set_comms_matrix(0);
    
    if (avr485_in != out_n)
	return -2;
    if (memcmp(avr485_inbuf, outbuf, out_n))
	return -3;
    return 0;
}


/*
 * Do all Tests
 */ 
 int test_juice_comms(int randrun)
{
    int passed, retries, anyfailed = 0;
    
    /* CONSOLE RS232 */
    retries = passed = 0;
    while ((!passed) && (retries++ < TEST_RETRIES))
	passed = !test_con232_egress(1);
    printf("CONSOLE RS232 EGRESS: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
    
    retries = passed = 0;
    while ((!passed) && (retries++ < TEST_RETRIES))
	passed = !test_con232_ingress(1);
    printf("CONSOLE RS232 INGRESS: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
    
    /* AVR RS232 */
    retries = passed = 0;
    while ((!passed) && (retries++ < TEST_RETRIES)) {
	passed = !test_avr232_egress(1);
	if (!passed) {
	    msleep(150);
	    printbufs();
	}
    }
    printf("AVR RS232 EGRESS: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
    
    retries = passed = 0;
    while ((!passed) && (retries++ < TEST_RETRIES)) {
	passed = !test_avr232_ingress(1);
	if (!passed) {
	    printbufs();
	    msleep(150);
	}
    }
    printf("AVR RS232 INGRESS: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
	   
    /* AVR RS485 */
    retries = passed = 0;
    while ((!passed) && (retries++ < TEST_RETRIES)) {
	passed = !test_avr485_egress(1);
	if (!passed)
	    printbufs();
    }
    printf("AVR RS485 EGRESS: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
	   
    retries = passed = 0;
    while ((!passed) && (retries++ < TEST_RETRIES)) {
	passed = !test_avr485_ingress(1);
	if (!passed)
	    printbufs();
    }
    printf("AVR RS485 INGRESS: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
	   
    return anyfailed;
}