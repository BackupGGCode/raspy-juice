#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib-juice.h"
#include "../firmware/juice.h"

int main(int argc, char *argv[])
{
    int i2caddr = DEFAULT_JUICE_I2CADDR;
    char *version;

    int count = 0;
    int i, rval, stat, b, adc_v;
    double volts = 0.0;
    
    int servo0pwm = 1500, servo0spddir = 50;
    int servo1pwm = 1500, servo1spddir = 100;
    int servo2pwm = 1500, servo2spddir = 150;
    int servo3pwm = 1500, servo3spddir = 200;

    printf("Hello, world!\n");
    
    rval = rj_open(i2caddr);
    if (rval < 0) {
	printf("juice: open failed, rval = %d\n", rval);
	exit(1);
    }
    else
	printf("juice: open at 0x%02x: succeeded.\n", i2caddr);

    version = rj_getversion();
    if (version != NULL)
	printf("juice: firmware version = %s\n", version);
    else {
	printf("juice: rj_getversion failed.\n");
	exit(2);
    }
    printf("\n");

    rj_setservo(SERVO_0, servo0pwm);
    usleep(500000);
    rj_setservo(SERVO_1, servo1pwm);
    usleep(500000);
    rj_setservo(SERVO_2, servo2pwm);
    usleep(500000);
    rj_setservo(SERVO_3, servo3pwm);
    usleep(500000);

    while (1) {

	count++;
	printf("\33[2K\r%06d:", count);
	
	stat = rj_readstat();
	if (stat >= 0) {
	  for (b = 7; b >= 0; b--)
	    printf("%c", (stat & (1 << b)) ? '1' : '0');
	  printf(": ");
	  fflush(stdout);
	  if (stat & ADCBUSY)
	    printf("adcbusy:");
	  if (stat & EEBUSY)
	    printf("eebusy:");
	  if (stat & RXA485)
	    printf("rs485:");
	  if (stat & RXA232)
	    printf("rs232:");
	}

#if 0
	if (stat & RXA485) {
	    rval = rj485_read();
	    printf("\nrs485: %s:\n", inbuf);
	    for (i = 0; inbuf[i] != 0; i++)
		printf("0x%02x ", inbuf[i]);
	    printf("\n");
		    
	}

	if (stat & RXA232) {
	    rval = rj232_read();
	    printf("\nrs232: %s: \n", inbuf);
	    for (i = 0; inbuf[i] != 0; i++)
		printf("0x%02x ", inbuf[i]);
	    printf("\n");
	}

	if ((count % 10) == 0) {
	    sprintf(outbuf, "Hello 232!!! count = %d\n\r", count);
	    rj232_send(outbuf, strlen(outbuf));
	    sprintf(outbuf, "Hello 485!!! count = %d\n\r", count); 
	    rj485_send(outbuf, strlen(outbuf));
	}
#endif
	
	adc_v = rj_readadc(0x47) & 0x3ff;
	volts = (3.3 * (10000 + 470 + 1000) / 1000) * adc_v / 0x3ff;
	printf("  ADC = 0x%04x, % 4d, %f ", adc_v, adc_v, volts);

	for(i = 0; i < 2 * volts; i++)
	    printf("*");

	fflush(stdout);


	servo0pwm += servo0spddir;
	if ((servo0pwm > 2500) || (servo0pwm < 500))
	    servo0spddir = -servo0spddir;
	rj_setservo(SERVO_0, servo0pwm);

	servo1pwm += servo1spddir;
	if ((servo1pwm > 2500) || (servo1pwm < 500))
	    servo1spddir = -servo1spddir;
	rj_setservo(SERVO_1, servo1pwm);

	servo2pwm += servo2spddir;
	if ((servo2pwm > 2500) || (servo2pwm < 500))
	    servo2spddir = -servo2spddir;
	rj_setservo(SERVO_2, servo2pwm);

	servo3pwm += servo3spddir;
	if ((servo3pwm > 2500) || (servo3pwm < 500))
	    servo3spddir = -servo3spddir;
	rj_setservo(SERVO_3, servo3pwm);

	usleep(100000);
    }
    
}

