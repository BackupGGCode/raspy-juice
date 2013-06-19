#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "juice-jig.h"

#define CODE_PRODUCT	"RJ2B"

#define COUNTERFILE	"sernum.txt"

#define CMD_AVRDUDE	"/usr/local/bin/avrdude -c linuxgpio -p m168 "
#define CMD_AVR_QUIET	"/usr/local/bin/avrdude -c linuxgpio -p m168 -qq "
#define REDIRECTION	" 2>&1 "

#define R_EFUSE		" -qq -U efuse:r:-:h"
#define R_HFUSE		" -qq -U hfuse:r:-:h"
#define R_LFUSE		" -qq -U lfuse:r:-:h"
#define R_FUSES3LINES	" -qq -U efuse:r:-:h -U hfuse:r:-:h -U lfuse:r:-:h"

#define W_EFUSE		" -U efuse:w:0x02:m"
#define W_HFUSE		" -U hfuse:w:0xD7:m"
#define W_LFUSE		" -U lfuse:w:0xE6:m"
#define W_ALLFUSES	" -c linuxgpio -p m168 -U efuse:w:0x02:m -U hfuse:w:0xD7:m -U lfuse:w:0xE6:m"

#define W_FLASH		" -U flash:w:twiboot-win.hex"
#define R_EEPROM	" -U eeprom:r:eep.raw:r"
#define W_EEPROM	" -U eeprom:w:eep-new.raw:r"
#define EE_SER_OFFSET	(512-16)

#define LINEBUFSZ	256
#define FMT_FULLDATE	"[%Y-%m-%d %H:%M:%S %Z]"

#define CMD_AVR_I2CRST 	"/usr/bin/i2cset -y 1  0x48 0xb0 0x0d "
#define CMD_TWIBOOT_I2CREV1	"/usr/local/bin/twiboot -d /dev/i2c-0 -a 0x29 -p 0 -w flash:juice.hex "
#define CMD_TWIBOOT_I2CREV2	"/usr/local/bin/twiboot -d /dev/i2c-1 -a 0x29 -p 0 -w flash:juice.hex "


char bigbuf[16384];

#ifdef	STANDALONE

#define LOGFILENAME	"jlog.txt"
int i2caddr_adc = 0x69;
int rpi_rev = 0;

int i2cbus_open(const char *devbusname)
{
    int rval;
    unsigned char i2c_buffer[16];

    /* test bus */
    fd_i2cbus = open(devbusname, O_RDWR);
    if (fd_i2cbus < 0)
	return -1;

    /* setup ADC device as slave*/
    fd_adc = ioctl(fd_i2cbus, I2C_SLAVE, i2caddr_adc);
    if (fd_adc < 0)
       return -2;
    
    /* read ADC device */
    rval = read(fd_i2cbus, i2c_buffer, 4);
    if (rval < 0)
	return -3;

    return fd_i2cbus;
}
#endif	/* STANDALONE */


int do_command(char *command, char *reply, int size)
{
    char cmdbuf[LINEBUFSZ], linebuf[LINEBUFSZ];
    FILE *procfile;
#ifdef STANDALONE
    FILE *logfile;
#endif
    int fail = 0, total_size = 0;
    time_t nowtime;
    struct tm *tm_info;
    char timestr[32];
    
    strcpy(cmdbuf, command);
    strcat(cmdbuf, " ");
    strcat(cmdbuf, REDIRECTION);
    printf("do_command(): popen'ing  '%s'\n", cmdbuf);

    if ((procfile = popen(cmdbuf, "r")) == NULL) {
	printf("do_command: shelling '%s' failed\n", cmdbuf);
	return -1;
    }
    
#ifdef STANDALONE
    if ((logfile = fopen(LOGFILENAME, "a")) == NULL) {
	printf("do_command: fopen '%s' failed\n", LOGFILENAME);
	fclose(procfile);
	return -1;
    }
#endif
    
    while (!feof(procfile)) {
	fgets(linebuf, LINEBUFSZ, procfile);
	
	time(&nowtime); /* this the line getting current time */
	tm_info = localtime(&nowtime);
	strftime(timestr, 32, "[%Y-%m-%d %H:%M] ", tm_info);

	fputs(timestr, logfile);
	fputs(linebuf, logfile);
	total_size += strlen(linebuf);
	if (total_size < (size - LINEBUFSZ)) {
	    /* strcat to *reply buf */
	}
	
	if (strcasestr(linebuf, "fail")) {
	    fail = 1;
	}
    }

    fclose(procfile);
#ifdef STANDALONE
    fclose(logfile);
#endif
    if (fail)
	return -total_size;
    return total_size;
}

int put_avr_fuses(void)
{
    return do_command(CMD_AVR_QUIET W_ALLFUSES, bigbuf, sizeof(bigbuf));
}

int put_avr_bootloader(char *filename)
{
    return do_command(CMD_AVR_QUIET W_FLASH, bigbuf, sizeof(bigbuf));
}

int put_avr_serial(char *sermem)
{
    FILE *neepfile;
    
    if ((neepfile = fopen("eep-new.raw", "w+")) == NULL) {
	printf("put_avr_serial: open eep-new.raw file failed\n");
	return -1;
    }
    fseek(neepfile, EE_SER_OFFSET, SEEK_SET);
    fwrite(sermem, 1, 16, neepfile);
    fclose(neepfile);
    
    return do_command(CMD_AVR_QUIET W_EEPROM, bigbuf, sizeof(bigbuf));
}

int get_avr_serial(char *sermem)
{
    FILE *eepfile;
    int r, fail = 0;
    
    r = do_command(CMD_AVR_QUIET R_EEPROM, bigbuf, sizeof(bigbuf));
    if (r < 0) {
	return -1;
    }
    
    if ((eepfile = fopen("eep.raw", "r")) == NULL) {
	printf("%s: open eep.raw file failed\n", __func__);
	fail = 1;
    }
    fseek(eepfile, 512-16, SEEK_SET);
    fread(sermem, 1, 16, eepfile);
    fclose(eepfile);
    sermem[16] = 0;

#if 0
    for (i = 0, p = serial_mem; i < 16; i++, p++) {
	if (i !=0 && (i % 8) == 0)   printf("\n");
	printf("%c 0x%02x ", isprint(*p) ? *p : '.', *p);
    }
    printf("\n");
#endif

    return 0;
}


int get_new_serial(char *filename, char *result)
{
    FILE *serialfile;
    int counter = 0;
    char serial[16];
    time_t nowtime;
    struct tm *tm_info;
    char workweek[32];
    
    if ((serialfile = fopen(filename, "r+")) == NULL) {
	printf("get_new_serial(): open %s failed.\n", filename);
	return -1;
    }
    if (fscanf(serialfile, "%d", &counter) == 0) {
	counter = 0;
    }
    fseek(serialfile, 0x0, SEEK_SET);
    fprintf(serialfile, "%d\n", ++counter);
    fclose(serialfile);
    snprintf(serial, 6, "%05d", counter);
    
    time(&nowtime); /* this the line getting current time */
    tm_info = localtime(&nowtime);
    strftime(workweek, 4+1, "%g%V", tm_info);
    
    strncpy(result, CODE_PRODUCT, 4+1);
    strcat(result, workweek);
    strcat(result, serial);
    strcat(result, "##");
    
    return counter;
}
    

int put_avr_firmware(char *firmware_filename)
{
    FILE *eepfile;
    int r, fail = 0;
    
    r = do_command(CMD_AVR_I2CRST, bigbuf, sizeof(bigbuf));
    if (r < 0) {
	printf("do_twiboot: CMD_AVR_I2CRST: failed.\n");
	/* no return, because in bootloader mode, 
	 AVR doesn't need I2C reset */
	/* return -1; */
    }
    usleep(100*1000L);

    if (rpi_rev == 1) {
	r = do_command(CMD_TWIBOOT_I2CREV1, bigbuf, sizeof(bigbuf));
    }
    else if (rpi_rev == 2) {
	r = do_command(CMD_TWIBOOT_I2CREV2, bigbuf, sizeof(bigbuf));
    }
    else {
	printf("put_avr_firmware: rpi_rev incorrectly, or not set\n");
    }
    
    /* DO STUFF TO CHECK twiboot SUCCESS/FAILURE FROM bigbuf*/
    
    return r;
}


int do_avr_run(void)
{
    char serial_mem[32], *p;
    int r, i, sernum;
    int randrun, run;

    r = get_avr_serial(serial_mem);
    if (r < 0) {
	printf("get_avr_serial: failed\n");
	return -1;
    }
    else 
	printf("get_avr_serial: successful %s\n", serial_mem);
    
    /* Should get & put a new serial number only if needed */
    r = get_new_serial(COUNTERFILE, serial_mem);
    if (r < 0) {
	printf("get_new_serial: failed\n");
	return -2;
    }
    else
	printf("get_new_serial: successful %s\n", serial_mem);
    
    r = put_avr_serial(serial_mem);
    if (r < 0) {
	printf("put_avr_serial: failed\n");
	return -3;
    }
    else
	printf("put_avr_serial: successful %s\n", serial_mem);
    
    if (put_avr_bootloader(NULL) < 0) {
	printf("put_avr_bootloader: failed\n");
	return -4;
    }
    else
	printf("put_avr_bootloader: successful\n");
    
    if (put_avr_fuses() < 0) {
	printf("put_avr_fuses: failed\n");
	return -5;
    }
    else
	printf("put_avr_fuses: successful\n");
    
    if (put_avr_firmware(NULL) < 0) {
	printf("put_avr_firmware: failed\n");
	return -6;
    }
    else
	printf("put_avr_firmware: successful\n");
    
    return 0;
}



#ifdef STANDALONE
int main(int argc, char *argv[])
{
    char serial_mem[32], *p;
    int r, i, sernum;
    int randrun, run;
    
    
    printf("Hello, world! Testing shelling out systems calls\n\n");

    /* Test of RPI-DAQ existence and Pi revision */
    rpi_rev = 1;
    fd_i2cbus = i2cbus_open("/dev/i2c-0");
    if (fd_i2cbus < 0) {
	rpi_rev = 2;
	fd_i2cbus = i2cbus_open("/dev/i2c-1");
    }
    if (fd_i2cbus < 0) {
	printf("i2cbus_open: /dev/i2c-x unsuccessful, DAQ not found.\n");
	return -1;
    }
    printf("i2cbus open: successful, rpi_rev = %d, fd_i2cbus = %d\n",
	   rpi_rev, fd_i2cbus);


    srand (time(NULL));
    randrun = (rand() % 5) + 1;
    randrun = 1;
    
    printf("Running a random run of %d\n", randrun);

    for (run = 0; run < randrun; run++) {
	printf("\nRUN #%d\n", run);
	do_avr_run();
    }
    
    printf("Test ended.\n");    
    return 0;   
}
#endif
