#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include "rpi-daq.h"

int main(int argc, char *argv[])
{
    int i, desired, count = 0;
    char lcd_s[32];
    
    printf("Hello, world!\n");
    printf("juice-jig main: Test app of Raspy Juice with RPi-DAQ and jig\n\n");
    
    if (rpi_daq_init()) {
	printf("juice-jig: rpi_daq_init() failed\n");
	return 1;
    }
    lcd_init(2, 20, 4);
    
    if (argc < 2)
	exit(0);
    
    desired = atoi(argv[1]);

    /* switch on relay 0 */
    daq_set_relay(0, 1);
    sleep(2);
    daq_set_buffered_i2c(1);

#if 1
    printf("Got out of lcdInit()\n");
    sleep(1);
    lcd_pos(0, 0);
    lcd_puts("Dave Appleton") ;
    lcd_pos(1, 0);
    lcd_puts("-------------") ;
    printf("Got out of lcd_main()\n");
#endif

    if (desired == 1) {
	printf("state = 1, setting and leaving buffered AVR interface on\n");
	daq_set_buffered_avr(1);
	/* leaving com port selection as RPICOM <-> AVR232 */
	daq_set_com_matrix(0x22);
	/* Hmmm, what does this do? */
	//daq_xfer(3, cfg_data);
	daq_set_relay(2, 1);
	sleep(1);
	daq_set_relay(3, 1);
	sleep(1);
	daq_set_relay(1, 1);
	exit(0);
    }
    

    if (desired == 2) {
	while (1) {
	    lcd_pos(1, 5);
	    sprintf(lcd_s, "Seconds = %d", count++);
	    lcd_puts(lcd_s);
	    sleep(1);

	}
    }

    
    
    /* turn off everything */
    daq_lcd_data(0x00);
    daq_lcd_regsel(0);
    for (i = 0; i < 4; i++) {
	daq_set_led(i, 0);
	daq_set_relay(i, 0);
    }
    daq_set_buffered_avr(0);
    daq_set_buffered_i2c(0);
    
    return 0;
}


