#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

#include "lib-juice.h"
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


int do_state_2_without(int dont_do_stuff);

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

#define ADC2VOLTS (11.0 / 1000000.0)

void adc_printall(float vals[4])
{
    mcp3424_readall(vals);
    
    printf("%s: %2.3f V\n", "CH1 DUT+5V-Main ", vals[0] * ADC2VOLTS);
    printf("%s: %2.3f V\n", "CH2 DUT+5V-Servo", vals[1] * ADC2VOLTS);
    printf("%s: %2.3f V\n", "CH3 DUT+3V-Aux  ", vals[2] * ADC2VOLTS);
    printf("%s: %2.3f V\n", "CH4 DUT Itotal  ", vals[3] * ADC2VOLTS);
    printf("\n");
}

static int limits(float lolimit, float val, float hilimit)
{
    if (val < lolimit)
	return -1;
    if (val > hilimit)
	return 1;
    return 0;
}

int test_juice_power(float vals[4])
{
    int i;
    for (i = 0; i < 4; i++)
	vals[i] = vals[i] * ADC2VOLTS;
    
    if (limits(4.75, vals[0], 5.27)) {
	printf("CH1 DUT+5V-Main out-of-range %2.3fV\n", vals[0]);
	return -1;
    }
    if (limits(4.75, vals[1], 5.27)) {
	printf("CH1 DUT+5V-Servo out-of-range %2.3fV\n", vals[1]);
	return -2;
    }
    if (limits(3.00, vals[2], 3.60)) {
	printf("CH1 DUT+3V-Aux out-of-range %2.3fV\n", vals[2]);
	return -3;
    }
#if 0
    if (limits(4.95, vals[3], 5.25)) {
	printf("CH1 DUT Itotal out-of-range %2.3fV\n", vals[3]);
	return -4;
    }
#endif
    return 0;
}
    
static int rj_initialize(void)
{   
    int r;
    char devbusname[] = "/dev/i2c-0";
    int i2caddr = AVRSLAVE_ADDR;
    char *version;
    
    r = rj_open(devbusname, i2caddr);
    if (r < 0) {
	printf("juice: open %s: failed, r = %d\n", devbusname, r);
	return -1;
    }
    else {
	printf("juice: open at 0x%02x: succeeded.\n", i2caddr);
    }
    
    version = rj_getversion();
    if (version == NULL) {
	printf("juice: rj_getversion: failed.\n\n");
	return -2;
    }
    else {
	printf("juice: firmware version = %s\n\n", version);
    }

    return 0;
}


void close_exit(int exitcode)
{
    int i;
    
    /* turn off everything */
    daq_lcd_data(0x00);
    daq_lcd_regsel(0);
    for (i = 0; i < 4; i++) {
	daq_set_led(i, 0);
	daq_set_relay(i, 0);
    }
    daq_set_buffered_avr(0);
    daq_set_buffered_i2c(0);
    rpi_comport_close();
    printf("\nmain: closed everything.\n\n");
    exit(exitcode);
}
    
    
int main(int argc, char *argv[])
{
    int desired;
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

    desired = 0;
    if (argc > 1)
	desired = atoi(argv[1]);
	

    /* Default LCD message */
    lcd_pos(0, 0);
    lcd_puts("Juice Jig Tester") ;
    lcd_pos(1, 0);
    lcd_puts("----------------") ;

    /* Just a quick turn-on for AVR programming development */
    if (desired == 1) {
	/* Turn on DUT power and settle */
	daq_set_relay(RELAY_VIN, 1);
	usleep(1000U * 200);

	/* Turn on load relays for +3V-AUX, +5V-SERVO, +5V-MAIN */
	daq_set_relay(RELAY_5VMAIN, 1);
	usleep(1000U * 200);

	daq_set_relay(RELAY_5VSERVO, 1);
	usleep(1000U * 200);

	daq_set_relay(RELAY_3VAUX, 1);
	usleep(1000U * 200);

	adc_printall(adc_vals);
	
	/* Turn on jig buffered connections */
	daq_set_buffered_i2c(1);
	daq_set_buffered_avr(1);

	lcd_clear();
	lcd_puts("JJ State 1");
	printf("Exiting as state = 1, power-on and leaving buffered AVR interface on\n");
	exit(0);
    }

    /* Do a one-run "full" test cycle ? */
    if (desired == 2) {

#define WAIT_FOR_BUTTON	0x0001
#define AVR_FLASHING	0x0002

	do_state_2_without(WAIT_FOR_BUTTON |
			   AVR_FLASHING);
    }
    
    if (desired == 3) {
	
	do_state_2_without(WAIT_FOR_BUTTON);
    }
    
    if (desired == 4) {
	/* Do loopy-loop of desired==2 one-time test
	 * with push button start, and halfway stopping
	 */
    }
    
    close_exit(0);
    return 0;
}

int do_state_2_without(int dont_do_stuff)
{
    int r;
    float adc_vals[4];
	
    if (!(dont_do_stuff & WAIT_FOR_BUTTON)) {
	printf("Waiting for keypress for one-time 'full' cycle...\n");
	/* fixme: refac into method daq_get_key */
	while ((daq_xfer(DAQ_SPISUB_NOP, 0) & 0x03) == 0)
	;
	printf("OK, got keypress\n");
    }

    /* Turn on DUT power and settle */
    daq_set_relay(RELAY_VIN, 1);
    usleep(1000U * 200);
//	adc_printall(adc_vals);

    /* Turn on DUT loads, settle and measure */
    daq_set_relay(RELAY_5VMAIN, 1);
    usleep(1000U * 200);
//	adc_printall(adc_vals);

    daq_set_relay(RELAY_5VSERVO, 1);
    usleep(1000U * 200);
//	adc_printall(adc_vals);

    daq_set_relay(RELAY_3VAUX, 1);
    usleep(1000U * 1000);
    adc_printall(adc_vals);
	
    /* fixme: test power levels */
    if (test_juice_power(adc_vals)) {
	printf("main: test_juice_power failed, exiting.\n");
	close_exit(-1);
    }
	
    /* Turn on DUT I2C buffered connection */
    daq_set_buffered_i2c(1);
    daq_set_buffered_avr(1);

    /* do AVR stuff */
    if (!(dont_do_stuff & AVR_FLASHING)) {
	r = do_avr_run();
	printf("main: do_avr_run r = %d\n", r);
    }
	
    /* fixme: bring Raspy Juice alive with lib-juice and set comms */
    if (rj_initialize()) {
	printf("main: rj_initialize() failed.\n");
	close_exit(-1);
    }

    /* fixme: open RPi com port with default 9600,8N1 */
    if (rpi_comport_open("/dev/ttyAMA0")) {
	printf("main: rpi_comport_open failed.\n");
	close_exit(-1);
    }
    printf("main: rpi_comport_open: succeeded, fd_comport = %d\n", 
	   fd_comport);

    /* fixme: do com ports test stuff */
    test_juice_comms(1);
	
    /* fixme: test of power LED, and AVR service firmware flashing */
	
    /* fixme: do pin port stuff */
	
    /* fixme: status report generation */

    return 0;
}
