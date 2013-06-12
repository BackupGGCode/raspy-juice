#include <stdio.h>
#include <unistd.h>
#include "mcp342x.h"



int mcp3424_readall(float *buf)
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

int main(void)
{
    int ch;
    float val[4];
    
    mcp342x_init();


#if 0
    mcp342x_set_config(MCP342X_ADC_CH4, MCP342X_GS_X1, MCP342X_ADC_RES_16, MCP342X_OC_CONTINUOUS);
    while(1) {
        // X4 gain, 16-bit res, Continuous
        mcp342x_request_conversion();
        sleep(1);
        float temp = mcp342x_read_output(MCP342X_ADC_CH4);
        printf("%3.2f uV\n", temp);
    }
#endif
    
    
    while (1) {
	mcp3424_readall(val);
	printf("%s: %3.2f uV\n", "CH1 DUT+5V-Main ", val[0] * 1.1);
	printf("%s: %3.2f uV\n", "CH2 DUT+5V-Servo", val[1] * 1.1);
	printf("%s: %3.2f uV\n", "CH3 DUT+3V-Aux  ", val[2] * 1.1);
	printf("%s: %3.2f uV\n", "CH4 DUT Icurrent", val[3] * 1.1);
	printf("\n");
	sleep(1);
    }
}
