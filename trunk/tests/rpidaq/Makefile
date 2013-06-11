###############################################################################
# Makefile for the juice-test
###############################################################################
CC=gcc
CFLAGS=-c -Wall -I../c_examples
LDFLAGS=

TARGET=jj

SOURCES=rpi-daq.c lib-lcd.c
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -rf $(TARGET) *.o


