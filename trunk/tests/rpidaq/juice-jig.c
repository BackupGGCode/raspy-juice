#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

#include "juice-jig.h"
#include "mcp342x.h"

FILE *logfile;
#define LOGFILENAME	"jlog.txt"

#define RELAY_VIN	0
#define RELAY_5VMAIN	3
#define RELAY_5VSERVO	2
#define RELAY_3VAUX	1

#define UI_SW1		0x01
#define UI_SW2		0x02
#define UI_RED		0x0010
#define UI_AMBER	0x0020
#define UI_GREEN	0x0040


void mcp3424_readall(float *buf)
{
    int ch;
    for (ch = 0; ch < 4; ch++) {
	mcp342x_set_config(ch, MCP342X_GS_X4, MCP342X_ADC_RES_16, 
			   MCP342X_OC_CONTINUOUS);
        mcp342x_request_conversion();
	usleep(1000L * 100);
	buf[ch] = mcp342x_read_output(ch);
    }
}

void adc_printall(float vals[4])
{
    mcp3424_readall(vals);
    
    printf("%s: %3.2f uV\n", "CH1 DUT+5V-Main ", vals[0] * 1.1);
    printf("%s: %3.2f uV\n", "CH2 DUT+5V-Servo", vals[1] * 1.1);
    printf("%s: %3.2f uV\n", "CH3 DUT+3V-Aux  ", vals[2] * 1.1);
    printf("%s: %3.2f uV\n", "CH4 DUT Itotal  ", vals[3] * 1.1);
    printf("\n");
}

int main(int argc, char *argv[])
{
    int i, desired, r;
    float adc_vals[4];
    
    printf("Hello, world!\n");
    printf("juice-jig main: Test app of Raspy Juice with RPi-DAQ and jig\n\n");
    
    if ((logfile = fopen(LOGFILENAME, "a")) == NULL) {
	printf("juice-jig: fopen '%s' failed\n", LOGFILENAME);
	return -1;
    }

    if (rpi_daq_init()) {
	printf("juice-jig: rpi_daq_init() failed\n");
	return 1;
    }
    lcd_init(2, 20, 4);
    //mcp342x_init(); using global rpi-daq::fd_i2cbus

    if (argc < 2)
	exit(0);
    
    /* my haha optargs */
    desired = atoi(argv[1]);

    /* switch on relay 0 */

    lcd_pos(0, 0);
    lcd_puts("Juice Jig Tester") ;
    lcd_pos(1, 0);
    lcd_puts("----------------") ;

    /* Just a quickie turn-on for AVR programming development */
    if (desired == 1) {
	/* Turn on DUT power and settle */
	daq_set_relay(RELAY_VIN, 1);
	usleep(1000U * 200);
	/* leaving com port selection as RPICOM <-> AVR232 */
	daq_set_com_matrix(0x22);

	/* Turn on load relays for +3V-AUX, +5V-SERVO, +5V-MAIN */
	daq_set_relay(RELAY_5VMAIN, 1);
	usleep(1000U * 200);

	daq_set_relay(RELAY_5VSERVO, 1);
	usleep(1000U * 200);

	daq_set_relay(RELAY_3VAUX, 1);
	usleep(1000U * 200);

	adc_printall(adc_vals);
	
	/* Turn on DUT I2C buffered connection */
	daq_set_buffered_i2c(1);
	daq_set_buffered_avr(1);

	lcd_clear();
	lcd_puts("JJ State 1");
	printf("Exiting as state = 1, power-on and leaving buffered AVR interface on\n");
	exit(0);
    }

    /* Do a one-run "full" test cycle ? */
    if (desired == 2) {
	
#ifdef IRRITATING_BUTTON_PUSH
	printf("Waiting for keypress for one-time 'full' cycle...\n");
	/* fixme: refac into method daq_get_key */
	while ((daq_xfer(DAQ_SPISUB_NOP, 0) & 0x03) == 0)
	    ;
	printf("OK, got keypress\n");
#endif
	/* Turn on DUT power and settle */
	daq_set_relay(RELAY_VIN, 1);
	usleep(1000U * 200);
	adc_printall(adc_vals);

	/* Turn on DUT loads, settle and measure */
	daq_set_relay(RELAY_5VMAIN, 1);
	usleep(1000U * 200);
	adc_printall(adc_vals);

	daq_set_relay(RELAY_5VSERVO, 1);
	usleep(1000U * 200);
	adc_printall(adc_vals);

	daq_set_relay(RELAY_3VAUX, 1);
	usleep(1000U * 200);
    	adc_printall(adc_vals);
	
	/* fixme: test power levels */
	
	/* Turn on DUT I2C buffered connection */
	daq_set_buffered_i2c(1);
	daq_set_buffered_avr(1);

	/* do AVR stuff */
	r = do_avr_run();
	printf("main: do_avr_run r = %d\n", r);
	
	/* fixme: test of power LED, and AVR service firmware flashing */
	
	/* fixme: do com port stuff */
	
	/* fixme: do pin port stuff */
	
	/* fixme: status report generation */
    
    }
    
    if (desired == 3) {
	/* Do loopy-loop of desired==2 one-time test
	 * with push button start, and halfway stopping
	 */
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


