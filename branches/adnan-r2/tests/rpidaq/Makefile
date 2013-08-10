###############################################################################
# Makefile for the juice-test
###############################################################################
CC = gcc
CFLAGS = -c -Wall -I../c_examples -DJUICE_JIG
LDFLAGS =

TARGET = jj

SOURCES = juice-jig.c juice-jig.h \
	rpi-daq.c rpi-daq.h \
	lib-lcd.c lib-juice.c lib-juice.h \
	mcp342x.h mcp342x.c \
	juice-comtests.c \
	jig-avr.c
	
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(TARGET) *.o *~




#### HAHAHAH
ball: svc-tests.elf svc-tests.hex 

bclean:
	rm -f svc-tests.elf svc-tests.hex *~

svc-tests.hex: svc-tests.elf
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature svc-tests.elf svc-tests.hex
#	avr-objcopy -O ihex svc-tests.elf svc-tests.hex

svc-tests.elf: svc-tests.c juice.h
	avr-gcc -mmcu=atmega168a -Wall -Os -gdwarf-2 -std=gnu99 \
	-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
	-DF_CPU=14745600UL -DSTANDALONE_UART_TEST \
	-o svc-tests.elf svc-tests.c

binstall: svc-tests.hex
	/usr/bin/i2cset -y 0 0x48 0xb0 0x0d ; sleep 0.5 ; \
	/usr/local/bin/twiboot -d /dev/i2c-0 -a 0x29 -w flash:svc-tests.hex

