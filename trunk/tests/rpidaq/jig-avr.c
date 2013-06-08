#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define LOGFILENAME	"jlog.txt"

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
#define W_ALLFUSES	" -qq -c linuxgpio -p m168 -U efuse:w:0x02:m -U hfuse:w:0xD7:m -U lfuse:w:0xE6:m"

#define W_FLASH		" -U flash:w:twiboot-win.hex"
#define R_EEPROM	" -U eeprom:r:eep.raw:r"
#define W_EEPROM	" -U eeprom:w:eep-new.raw:r"
#define SERNUM_OFFSET	(512-16)

#define LINEBUFSZ	256
#define FMT_FULLDATE	"[%Y-%m-%d %H:%M:%S %Z]"

static char *strtolower(char *src) 
{
    for ( ; *src; ++src) *src = tolower(*src);
}

int do_avrdude(char *cmd)
{
    printf("opening popen procfile '%s'\n", cmd);
    return 0;
}

int get_avr_serial(char *sertxt)
{
    FILE *procfile, *eepfile;
    char linebuf[LINEBUFSZ];
    char cmd[] = CMD_AVRDUDE R_EEPROM REDIRECTION;
    int fail = 0, i;
    char serbuf[20], *p;
    
    if ((procfile = popen(CMD_AVRDUDE R_EEPROM REDIRECTION, "r")) == NULL) {
	printf("popen procfile of '%s' failed\n");
	return -1;
    }
    
    while (!feof(procfile)) {
	fgets(linebuf, LINEBUFSZ, procfile);
	printf("get_avr_serial: %s", linebuf);
	if (strcasestr(linebuf, "fail") != NULL) {
	    fail = 1;
	}
    }

    if (fail) {
	printf("get_avr_serial: %s failed\n", cmd);
	fclose(procfile);
	return -1;
    }
    
    if (!fail && (eepfile = fopen("eep.raw", "r")) == NULL) {
	printf("%s: open eep.raw file failed\n", __func__);
	fail = 1;
    }

    fseek(eepfile, 512-16, SEEK_SET);
    fread(serbuf, 1, 16, eepfile);
    
    fclose(eepfile);
    fclose(procfile);
    return 0;
}

int put_avr_serial(char *sertxt)
{
    return 0;
}

int do_avr_fuses(void)
{
    FILE *procfile;
    char linebuf[LINEBUFSZ];
    int fail = 1;
    
    if ((procfile = popen(CMD_AVRDUDE W_ALLFUSES REDIRECTION, "r")) == NULL) {
	printf("popen procfile of '%s' failed\n");
	return -1;
    }
    
    while (!feof(procfile)) {
	fgets(linebuf, LINEBUFSZ, procfile);
	printf("do_avr_fuses: %s", linebuf);
	if (strcasestr(linebuf, "fail")) {
	    fail = 1;
	}
    }
    return fail;
}

int do_avr_flash(void)
{
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *logfile, *procfile;
    char linebuf[LINEBUFSZ];
    time_t nowtime;
    struct tm *tm_info;
    char timestr[32];
    char sernum_str[32], *p;
    int i;
    
    
    printf("Hello, world! Testing shelling out systems calls\n\n");

    get_avr_serial(sernum_str);
    for (i = 0, p = sernum_str; i < 16; i++, p++) {
	if (i !=0 && (i % 8) == 0)
	    printf("\n");
	printf("%c 0x%02x ", isprint(*p) ? *p : '.', *p);
    }
    printf("\n");
    
#if 0    
    for (i = 0; i < 5; i++) {
	
	printf("Run #%d:\n", i);

	if ((logfile = fopen(LOGFILENAME, i == 0 ? "w" : "a" )) == NULL) {
	    printf("fopen logfile failed\n");
	    exit(1);
	}
	
	if ((procfile = popen(CMD_AVRDUDE REDIRECTION, "r")) == NULL) {
	    printf("popen procfile failed\n");
	    exit(1);
	}
	
	
	while (!feof(procfile)) {
		fgets(linebuf, LINEBUFSZ, procfile);
	    
	    time(&nowtime); /* this the line getting current time */
	    tm_info = localtime(&nowtime);
	    strftime(timestr, 32, "[%Y-%m-%d %H:%M] ", tm_info);
	    fputs(timestr, logfile);
	    
	    fprintf(logfile, "Run #%d: ", i);
	    fputs(linebuf, logfile);
	}
	
	fclose(procfile);
	fclose(logfile);
	sleep(1);
    }
    printf("Test ended.\n");    
#endif

    
    return 0;   
}

