###############################################################################
# Makefile for the juice-test
###############################################################################
CC=gcc
CFLAGS=-c -Wall -I../c_examples
LDFLAGS=

TARGET=jj

SOURCES=juice-jig.c rpi-daq.c rpi-daq.h lib-lcd.c lib-juice.c lib-juice.h
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(TARGET) *.o *~


