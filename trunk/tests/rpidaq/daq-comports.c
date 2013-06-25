#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "juice-dev.h"

#define BUFSIZE 64
char avr_inbuf[BUFSIZE+1];
char avr_outbuf[BUFSIZE+1];
char rpi_inbuf[BUFSIZE+1];
char rpi_outbuf[BUFSIZE+1];

/* From tldp serial programming example */
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyAMA0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
static int fd_ttyAMA0;
static struct termios oldtio;
volatile int STOP=FALSE; 
void signal_handler_IO (int status);   /* definition of signal handler */
int wait_flag=TRUE;                    /* TRUE while no signal received */

void tldp_com_example_init(void);
void tldp_com_example_loop(void);
void tldp_com_example_exit(void);

void tldp_com_example_init(void)
{
    struct termios newtio;
    struct sigaction saio;           /* definition of signal action */
    
    /* From http://www.tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html#AEN129 */
    /* open the device to be non-blocking (read will return immediatly) */
    fd_ttyAMA0 = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ttyAMA0 < 0) {
	perror(MODEMDEVICE);
	exit(-1);
    }
   
    /* save current port settings */
    tcgetattr(fd_ttyAMA0, &oldtio);
    /* set new port settings for canonical input processing */
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    /* minimum one character, or 1 character timeout */
    newtio.c_cc[VMIN]=1;
    newtio.c_cc[VTIME]=1;
    tcflush(fd_ttyAMA0, TCIFLUSH);
    tcsetattr(fd_ttyAMA0, TCSANOW, &newtio);
}


void tldp_com_example_loop(void)
{
    int c, res;
    char buf[255];

    /* loop while waiting for input. normally
     we would do something useful here */ 
    while (STOP==FALSE) {
	printf(".\n");
	usleep(100000);
	/* after receiving SIGIO, wait_flag = FALSE, input is available
	 and can be read */
	if (wait_flag==FALSE) { 
	    res = read(fd_ttyAMA0, buf, 255);
	    buf[res]=0;
	    printf(":%s:%d\n", buf, res);
	    /* stop loop if only a CR was input */
	    if (res==1)
		STOP=TRUE; 
	    /* wait for new input */
	    wait_flag = TRUE;
	}
    }
}

void tldp_com_example_exit(void)
{
    /* restore old port settings */
    tcsetattr(fd_ttyAMA0, TCSANOW, &oldtio);
}

/***************************************************************************
 * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
 * characters have been received.                                           *
 ***************************************************************************/
        
void signal_handler_IO (int status)
{
    printf("received SIGIO signal.\n");
    wait_flag = FALSE;
}


int test_avr232_comms(int randrun)
{
    char devbusname[] = "/dev/i2c-1";
    int i2caddr = AVRSLAVE_ADDR;
    char *version;

    int run, r, baud, stat;
    int retries = 0, avr_n = 0, rpi_n = 0;
    
    
    printf("Hello, world! I am example3 - reading/writing data from AVR-RS232\n");
    
    r = rj_open(devbusname, i2caddr);
    if (r < 0) {
	printf("open %s: failed, r = %d\n", devbusname, r);
    }
    else {
	printf("juice: open at 0x%02x: succeeded.\n", i2caddr);
    }
    
    version = rj_getversion();
    if (version != NULL)
	printf("juice: firmware version = %s\n", version);
    else {
	printf("juice: rj_getversion failed.\n");
    }

    baud = 9600;
    if (rj232_setbaud(baud)) {
	printf("rj232: set baud failed.\n");
    }
    else {
	printf("rj232: set baud to %dbps succeeded.\n", baud);
    }

    tldp_com_example_init();
    
    for (run = 0; run < randrun; run++) {


#if 1
	sprintf(avr_outbuf, "From AVR to RPi >>>>> run = %d\n\r", run);
	rj232_send((unsigned char *)avr_outbuf, strlen(avr_outbuf));
	printf("rj232_send: %d bytes sent.\n", strlen(avr_outbuf));
#endif       
	usleep(100000L);

#if 1
	sprintf(rpi_outbuf, "From RPi to AVR <<< run = %d\n\r", run);
	r = write(fd_ttyAMA0, rpi_outbuf, strlen(rpi_outbuf));
	printf("write(ttyAMA0): %d bytes sent.\n", r);
#endif       
	
	usleep(100000L);
	
	while (retries++ < 5) {
	
	    stat = rj_readstat();
	    
	    while(rj_readstat() & RXA232) {
		r = rj232_read((unsigned char *)avr_inbuf, BUFSIZE);
		avr_n += r;
	    }

	    r = read(fd_ttyAMA0, rpi_inbuf, 255);
	    rpi_inbuf[r] = 0;
	    rpi_n += r;

	    usleep(1000);
	}
	printf("avr_inbuf: %02d bytes: %s\n", avr_n, avr_inbuf);
	printf("rpi_inbuf: %02d bytes: %s\n", rpi_n, rpi_inbuf);
#if 0
	for (i = 0; avr_inbuf[i] != 0; i++)
	    printf("0x%02x ", avr_inbuf[i]);
	printf(": %s \n", avr_inbuf);
#endif
    }
    
    tldp_com_example_exit();
    close(fd_ttyAMA0);
}


