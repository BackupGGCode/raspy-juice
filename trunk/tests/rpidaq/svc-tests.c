/***********************************************************************
 * rs232-swuart.c
 * RS232 software UART driver for Raspy Juice (Raspberry Pi Exp Board)
 * Half-duplex, 9600, 8N1. Hardware pins based on juice.h
 * MCU: ATmega168A, 14.7456MHz
 *
 *
 * Copyright (c) 2012, Adnan Jalaludin <adnan singnet.com.sg>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

#include "juice.h"
#include <avr/interrupt.h>

/* With MCU clock of 14.7456MHz prescaled by 256, period = 34.7us */
#define DEFAULT_BPS9600_PRESCALER	0b100   // clk/64
#define DEFAULT_BPS9600_FULLPERIOD	24	// 104us

#define PD2_INT0_ENABLE()	{ EIFR |= (1<<INTF0);  EIMSK |=  (1<<INT0); }
#define PD2_INT0_DISABLE()	{ EIMSK &= ~(1<<INT0); }
#define TIMER2_INT_ENABLE()	{ TIFR2 = 0b111;       TIMSK2 = (1<<OCIE2A); }
#define TIMER2_INT_DISABLE()	{ TIMSK2 = 0; }

enum SWUART_STATES {
    IDLE = 0,
    RECV_START = 1,
    XMIT_START = 16,
};

static volatile char swuart_state = IDLE;
static volatile int  swuart_recv_reg;
static volatile int  swuart_xmit_reg;
static volatile char rxhead, rxtail, txhead, txtail;

static volatile uint8_t prescalar; 
static volatile uint8_t fullperiod;
static volatile uint8_t onehalfperiod;

#define RS232_RX_BUFSIZE 64
#define RS232_TX_BUFSIZE 64
#define RS232_RX_BUFMASK (RS232_RX_BUFSIZE - 1)
#define RS232_TX_BUFMASK (RS232_TX_BUFSIZE - 1)
#if    (RS232_RX_BUFSIZE & RS232_RX_BUFMASK)
#  error RS232 RX buffer size is not a power of 2
#endif
#if    (RS232_TX_BUFSIZE & RS232_TX_BUFMASK)
#  error RS232 TX buffer size is not a power of 2
#endif
unsigned char rs232_rxbuf[RS232_RX_BUFSIZE];
unsigned char rs232_txbuf[RS232_TX_BUFSIZE];

void rs232_swuart_init(void)
{
    rxhead = rxtail = txhead = txtail = 0;

    /* enable falling-edge INT0 on PD2 pin */
    EICRA |= ((1<<ISC01) | (0<<ISC00));
    /* set timer2 clear-on-compare-match mode */
    rs232_swuart_setbaud(DEFAULT_BPS9600_PRESCALER, DEFAULT_BPS9600_FULLPERIOD);

    swuart_state = IDLE;
    RS232_TXD_HIGH();
    TIMER2_INT_DISABLE();
    PD2_INT0_ENABLE();
}

void rs232_swuart_setbaud(uint8_t clock_prescale, uint8_t period_ticks)
{
    TCCR2A = (1<<WGM21) | (0<<WGM20);
    TCCR2B = clock_prescale & 0b00000111;

    fullperiod = period_ticks - 1;
    /* one and a half multiplication of fullperiod */
    onehalfperiod = period_ticks + (period_ticks >> 1);
    OCR2A = fullperiod;
}

void rs232_swuart_off(void)
{
    TIMER2_INT_DISABLE();
    PD2_INT0_DISABLE();
}

void rs232_putc(char c)
{
    unsigned char tmphead, tmptail;
    tmphead = (txhead + 1) & RS232_TX_BUFMASK;
    while (tmphead == txtail)
	;
    rs232_txbuf[tmphead] = c;
    txhead = tmphead;

    /* if SWUART is idle, need to pump state machine, else let
     * the interrupt state machine handle the transmission */

    if (swuart_state == IDLE) {
	PD2_INT0_DISABLE();

	tmptail = (txtail + 1) & RS232_TX_BUFMASK;
	txtail = tmptail;
	/* stuff in high stop bits */
	swuart_xmit_reg = rs232_txbuf[tmptail] | 0xf00;
	swuart_state = XMIT_START;
	RS232_TXD_LOW();
	TCNT2 = 0;
	TIMER2_INT_ENABLE();
    }
}

char rs232_havechar(void)
{
    return (rxhead != rxtail);
}

char rs232_getc(void)
{
    unsigned char tmptail;
    while (rxhead == rxtail)
	;
    tmptail = (rxtail + 1) & RS232_RX_BUFMASK;
    rxtail = tmptail;
    return rs232_rxbuf[tmptail];

}

int rs232_getchar(FILE *stream)
{
    return rs232_getc();
}

int rs232_putchar(char c, FILE *stream)
{
    if (c == '\n')
	rs232_putc('\r');
    rs232_putc(c);
    return 0;
}

void rs232_puts(char *s)
{
    while(*s)
	rs232_putchar(*s++, NULL);
}


/***********************************************************************
 * SWUART RxD Interrupts, Char and Packet Reception.
 ***********************************************************************/

ISR(INT0_vect)
{
    TCNT2 = 0;
    OCR2A = onehalfperiod;
    TIMER2_INT_ENABLE();
    if (swuart_state == IDLE) {
	swuart_state = RECV_START;
	swuart_recv_reg = 0;
	PD2_INT0_DISABLE();
    }
}

ISR(TIMER2_COMPA_vect)
{
    unsigned char tmptail;
    unsigned char tmphead;


    OCR2A = fullperiod;
    
    /* do nothing if in IDLE state */
    if (swuart_state == IDLE)
	return;
    
    if (swuart_state < XMIT_START) {
	// PD5_ON();
	/* process state machine for reception */
	if (swuart_state < 9) {
	    swuart_recv_reg >>= 1;
	    if (RS232_RXD())
		swuart_recv_reg |= 0b10000000;
	    swuart_state++;
	}
	else {
	    /* end reception */
	    TIMER2_INT_DISABLE();
	    PD2_INT0_ENABLE();
	    swuart_state = IDLE;

	    tmphead = (rxhead + 1) & RS232_RX_BUFMASK;
	    rxhead = tmphead;
	    if (tmphead == rxtail) {
		/* error! recv buffer overflow routine */
	    }
	    rs232_rxbuf[tmphead] = swuart_recv_reg;
	}
    }
    else {
	/* swuart state here is bigger than XMIT_START, */
	/* so state machine should be for transmission  */
	/* (XMIT_START + 10) includes high stop bit	*/

	if (swuart_state < (XMIT_START + 9)) {

	    if (swuart_xmit_reg & 0x01) {
		RS232_TXD_HIGH();
	    }
	    else {
		RS232_TXD_LOW();
	    }
	    swuart_xmit_reg >>= 1;
	    swuart_state++;
	}
	else {
	    /* transmission of stop bit completed */
	    TIMER2_INT_DISABLE();
	    PD2_INT0_ENABLE();
	    swuart_state = IDLE;
	}
    }

    /* Reception/Transmission has completed,
     * check if any data for transmission */
    if ((swuart_state == IDLE) && (txhead != txtail)) {
	tmptail = (txtail + 1) & RS232_TX_BUFMASK;
	txtail = tmptail;
	swuart_xmit_reg = rs232_txbuf[tmptail] | 0xf00;
	swuart_state = XMIT_START;
	RS232_TXD_LOW();
	TCNT2 = 0;
	TIMER2_INT_ENABLE();
    }
}


#ifdef STANDALONE_UART_TEST

/*
 * compile with:
avr-gcc -mmcu=atmega168a -Wall -Os -gdwarf-2 -std=gnu99 
  -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
  -DF_CPU=14745600UL -DSTANDALONE_UART_TEST \
  -o svc-tests.elf svc-tests.c
avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature \
  svc-tests.elf svc-tests.hex
 */
int main(void)
{
    char c;
    
    JUICE_PCBA_PINS_INIT();
    rs232_swuart_init();
    sei();
    rs232_puts("\nTest Application of SW-UART\n\n");
    
    while (1) {
	/* Testing rs232 echo */
	if (rs232_havechar()) {
	    c = rs232_getc();
	    if (c == '\r') {
		rs232_putc('\n');
	    }
	    rs232_putc(c);
	}
    }
}
#endif

 