#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "rpi-daq.h"
#include "lib-juice.h"

#define msleep(MS)	usleep(1000 * (MS))

#define CT_BUFSIZE 255
char rpi_inbuf[CT_BUFSIZE + 1];
char rpi_outbuf[CT_BUFSIZE + 1];
char avr_inbuf[CT_BUFSIZE + 1];
char avr_outbuf[CT_BUFSIZE + 1];
char avr232_inbuf[CT_BUFSIZE + 1];
char avr232_outbuf[CT_BUFSIZE + 1];
char avr485_inbuf[CT_BUFSIZE + 1];
char avr485_outbuf[CT_BUFSIZE + 1];

static void rj_flush(void)
{
    int retries;
    
    /* flush out rj232 and rj485 buffers */
    retries = 10;
    while (retries-- && (rj_readstat() & RXA232)) {
	rj232_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
    }
    retries = 10;
    while (retries-- && (rj_readstat() & RXA485)) {
	rj485_read((unsigned char *)avr_inbuf, CT_BUFSIZE);
    }
}
    
    

/*
 * Reads all RPi comport, AVR232 and AVR485 buffers.
 * Returns number of bytes in each inbuf by integer refs.
 * Returns number of retries to complete.
 */

#define NUM_RETRY 10

static int read_inbufs(int *rpi, int *avr232, int *avr485)
{
    int retries, stat, r, rpi_n, avr232_n, avr485_n;
    
    memset(rpi_inbuf, 0, CT_BUFSIZE);
    memset(avr232_inbuf, 0, CT_BUFSIZE);
    memset(avr485_inbuf, 0, CT_BUFSIZE);


    retries = NUM_RETRY;
    rpi_n = avr232_n = avr485_n = 0;
    
    while (retries--) {
	
	stat = rj_readstat();
	
	if (stat & RXA232) {
	    r = rj232_read((unsigned char *)avr232_inbuf + avr232_n, 
			   CT_BUFSIZE - avr232_n -1);
	    avr232_n += r;
	}
	
	if (stat & RXA485) {
	    r = rj485_read((unsigned char *)avr485_inbuf + avr485_n,
			   CT_BUFSIZE - avr485_n - 1);
	    avr485_n += r;
	}
	    
	r = read(fd_comport, rpi_inbuf + rpi_n, CT_BUFSIZE - rpi_n - 1);
	if (r > 0) {
	    rpi_n += r;
	}
	msleep(10);
    }
    
    rpi_inbuf[rpi_n] = 0;
    avr232_inbuf[avr232_n] = 0;
    avr485_inbuf[avr485_n] = 0;

    if (rpi)
	*rpi = rpi_n;
    if (avr232)
	*avr232 = avr232_n;
    if (avr485)
	*avr485 = avr485_n;
    
    printf("   rpi_inbuf: %02d bytes: %s\n", rpi_n, rpi_inbuf);
    printf("avr232_inbuf: %02d bytes: %s\n", avr232_n, avr232_inbuf);
    printf("avr485_inbuf: %02d bytes: %s\n", avr485_n, avr485_inbuf);
    
    return NUM_RETRY - retries;    
}

/*
 * Tests the auxiliary RS232 interface of the AVR microcontroller on Juice
 * The interface is a HALF Duplex, so it cannot simultaneously send/receive
 * data. A wait in between the transactions is neccessary.
 */
int test_avr232_comms(int randrun)
{
    int out_n, rpi_in, avr232_in;
    
    rpi_comport_setbaud(B9600);
    
    if (rj232_setbaud(9600)) {
	printf("%s: rj232_setbaud failed.\n", __func__);
	return -1;
    }
    /* AVR RS232 Egress TEST */
    printf("AVR RS232 Egress Test\n");
    sprintf(avr_outbuf, "RPi I2C >>>>> AVR RS232 TX >>>>> RPi RxD");
    out_n = strlen(avr_outbuf);
    
    daq_set_comms_matrix(RPIRXD_FROM_AVR232_TX);

    rj_flush();

    rj232_send((unsigned char *)avr_outbuf, out_n);
    
    read_inbufs(&rpi_in, &avr232_in, 0);

    if (rpi_in != out_n) {
	printf("      outbuf: %02d bytes: %s\n", out_n, avr_outbuf);
	printf("avr232 egress failed bytes: out %d in %d\n", out_n, rpi_in);
	return -2;
    }
    if (memcmp(rpi_inbuf, avr_outbuf, out_n)) {
	printf("      outbuf: %02d bytes: %s\n", out_n, avr_outbuf);
	printf("avr232 egress failed bytes: out %d in %d\n", out_n, rpi_in);
	return -3;
    }
    
    /* AVR RS232 Ingress TEST */
    printf("AVR RS232 Ingress Test\n");
    sprintf(rpi_outbuf, "RPi I2C <<<<<< AVR RS232 RX <<<<<< RPi Txd");
    out_n = strlen(avr_outbuf);
    
    daq_set_comms_matrix(RPITXD_TO_AVR232_RX);

    rj_flush();

    msleep(100);
    
    write(fd_comport, rpi_outbuf, out_n);
    
    msleep(100);

    read_inbufs(&rpi_in, &avr232_in, 0);

    msleep(100);
    
    if (rpi_in != out_n) {
	printf("      outbuf: %02d bytes: %s\n", out_n, rpi_outbuf);
	printf("avr232 ingress failed bytes: out %d in %d\n", out_n, avr232_in);
	return -2;
    }
    if (memcmp(avr_inbuf, rpi_outbuf, out_n)) {
	printf("      outbuf: %02d bytes: %s\n", out_n, rpi_outbuf);
	printf("avr232 ingress failed bytes: out %d in %d\n", out_n, avr232_in);
	return -3;
    }

    return 0;


}

/*
 * Tests the Raspberry Pi console connections from GPIO header to RS232-level
 * header pin, and from the RS232-level Rx pin to the GPIO header.
 */ 
int test_con232_comms(int randrun)
{
    int out_n, rpi_in;
    
    rpi_comport_setbaud(B115200);

    /* Console RS232 Egress */
    printf("Console RS232 Egress Test\n");
    sprintf(rpi_outbuf, "RPi Header TxD >>sayHello>>> Console RS232 out");
    out_n = strlen(rpi_outbuf);
    
    daq_set_comms_matrix(RPITXD_TO_DUT_HDR_TXD | RPIRXD_FROM_CONSOLE_TX);
    
    write(fd_comport, rpi_outbuf, out_n);
    
    read_inbufs(&rpi_in, 0, 0);

    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, rpi_outbuf, out_n))
	return -3;


    /* Console RS232 Ingress */
    printf("Console RS232 Ingress Test\n");
    sprintf(rpi_outbuf, "RPi Header Rxd <<someReply<<< Console RS232 in");
    out_n = strlen(rpi_outbuf);
    
    daq_set_comms_matrix(RPITXD_TO_CONSOLE_RX | RPIRXD_FROM_DUT_HDR_RXD);
    
    write(fd_comport, rpi_outbuf, out_n);
    
    read_inbufs(&rpi_in, 0, 0);
            
    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, rpi_outbuf, out_n))
	return -3;

    return 0;
}

/*
 * Tests the auxiliary RS485 interface of the AVR microcontroller on Juice
 * The interface is a HALF Duplex, so it cannot simultaneously send/receive
 * data. A wait in between the transactions is neccessary.
 */
int test_avr485_comms(int randrun)
{
    int out_n, rpi_in, avr485_in;
    
    rpi_comport_setbaud(B115200);
    
    if (rj485_setbaud(115200)) {
	printf("%s: rj485_setbaud failed.\n", __func__);
	return -1;
    }
    
    /* AVR RS485 Egress TEST */
    printf("AVR RS485 Egress Test\n");
    sprintf(avr_outbuf, "RPi >> I2C >> DUT RS485 >> xAA55aa55x >> RPi RS485 RxD");
    out_n = strlen(avr_outbuf);
    
    daq_set_comms_matrix(RPIRXD_FROM_AVR485_TX);

    rj_flush();

    rj485_send((unsigned char *) avr_outbuf, out_n);
    
    read_inbufs(&rpi_in, 0, &avr485_in);

    if (rpi_in != out_n)
	return -2;
    if (memcmp(rpi_inbuf, avr_outbuf, out_n))
	return -3;
    
    /* AVR RS485 Ingress TEST */
    printf("AVR RS485 Ingress Test\n");
    sprintf(rpi_outbuf, "RPi >> RS485 TxD >> xAA55aa55x >> DUT RS485 >> I2C >> RPi");
    out_n = strlen(rpi_outbuf);
    
    daq_set_comms_matrix(RPITXD_TO_AVR485_RX);

    rj_flush();

    write(fd_comport, rpi_outbuf, out_n);

    read_inbufs(&rpi_in, 0, &avr485_in);
    
    daq_set_comms_matrix(0);
    
    if (avr485_in != out_n)
	return -2;
    if (memcmp(avr485_inbuf, rpi_outbuf, out_n))
	return -3;
    
    return 0;
}


#define NUM_RETRY_UNTIL_PASS 10

int test_juice_comms(int randrun)
{
    int passed, retries, anyfailed;
    
    anyfailed = 0;
    retries = passed = 0;
    while ((!passed) && (retries++ < NUM_RETRY_UNTIL_PASS))
	   passed = !test_con232_comms(1);
    printf("CONSOLE RS232: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
	   
    retries = passed = 0;
    while ((!passed) && (retries++ < NUM_RETRY_UNTIL_PASS))
	   passed = !test_avr232_comms(1);
    printf("AVR RS232: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
	   
	   
    retries = passed = 0;
    while ((!passed) && (retries++ < NUM_RETRY_UNTIL_PASS))
	   passed = !test_avr485_comms(1);
    printf("AVR RS485: %s, RETRIES: %d\n", passed ? "Passed" : "Failed", retries - 1);
    if (!passed)
	anyfailed = 1;
	   
#if 0    
    if (test_con232_comms(1))
	return -1;
    if (test_avr232_comms(1))
	return -2;
    if (test_avr485_comms(1))
	return -3;
#endif
    return anyfailed;
}