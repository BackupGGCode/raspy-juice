#ifndef JUICE_JIG_H
#define JUICE_JIG_H

#include "rpi-daq.h"

extern FILE *logfile;

extern int do_avr_run(void);
extern int test_juice_comms(int randrun);
extern int test_avr232_comms(int randrun);
extern int test_con232_comms(int randrun);


#endif /* JUICE_JIG_H */