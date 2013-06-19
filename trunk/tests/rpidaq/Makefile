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
	jig-avr.c
	
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(TARGET) *.o *~
