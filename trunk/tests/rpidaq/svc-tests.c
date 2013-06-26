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
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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
FILE rs232_stream = FDEV_SETUP_STREAM(rs232_putchar, rs232_getchar, 
				      _FDEV_SETUP_RW);
char const VERSION_STR[] PROGMEM = "$Id:";
volatile static unsigned char twi_state = 0;
unsigned char led_state = 1;
long led_counter = 0, led_timing[4] = { 80000L, 40000L, 20000L, 40000L }; 

void led_heartbeat(void)
{

    if (led_counter++  > led_timing[led_state]) {
	led_counter = 0;
	led_state = ++led_state % 4;
	if (led_state % 2)
	    LED_ON();
	else
	    LED_OFF();
    }
}

#ifdef TWI_POLL_MODE
# define TWI_debug(fmt, ...)	printf_P(fmt, ##__VA_ARGS__)
#else
# define TWI_debug(fmt, ...)
#endif

int main(void)
{
    char c;
    
    JUICE_PCBA_PINS_INIT();
    
    rs232_swuart_init();
#if 0
    rs485_init();
    servo_init();
#endif
    /* Enable ADC, and set clock prescaler to div 128 */
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    
    TWAR = (AVRSLAVE_ADDR << 1);
    TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
    
    sei();

    /* Test printouts */
    stdout = stdin = &rs232_stream;
    TWI_debug("\r\nTest Application of Juice Firmware\r\n");
    TWI_debug("Second line\r\n\r\n");
    

    
    
    while (1) {
#if 1
	led_heartbeat();
	
	
	
	
	
#else
	/* Testing rs232 echo */
	if (rs232_havechar()) {
	    c = rs232_getc();
	    if (c == '\r') {
		rs232_putc('\n');
	    }
	    rs232_putc(c);
	}
#endif
    }
}


ISR(TWI_vect)
{
    static unsigned char bcnt, reg, prep_data, lobyte;
    static int servo_pwm, eeaddr;
    static const char *pgm_ptr;
    unsigned char data;
    
    twi_state = TWSR;
    switch (twi_state & 0xF8) {
	/* TWI SLAVE REGISTER READS */
    case TWI_STX_ADR_ACK:
    case TWI_STX_DATA_ACK:
	TWDR = prep_data;
	TWCR = (1<<TWEN)| (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	if (reg == ADCDAT) {
	    prep_data = ADCH;
	}
#if 0
	// TODO: have to re-think how to slave-xmit multiple bytes for
	// received data with ack/nack. Can't put the below lines because
	// if master asks for 1 byte only, pre-unload of reception FIFO's
	// will lose the data. So, for now, the master has to check GSTAT,
	// followed by single byte read -- a bit inefficient.
	
	else if (reg == RS232D) {
	    if (rs232_havechar())
		prep_data = rs232_getc();
	}
	else if (reg == RS485D) {
	    if (rs485_havechar())
		prep_data = rs485_getc();
	}
#endif
	else if (reg == EEDATA) {
#if 0	    
	    prep_data = eeprom_read_byte((unsigned char *)eeaddr++);
#endif
	}
	else if (reg == RJ_VERSION) {
	    prep_data = pgm_read_byte(pgm_ptr++);
	}
	break;

    case TWI_STX_DATA_NACK:
	// Data byte in TWDR has been transmitted; NACK has been received. 
	// Do nothing
	// Reset the TWI Interupt to wait for a new event.
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO)| (0<<TWWC);
	break;     

    case TWI_SRX_GEN_ACK:
    case TWI_SRX_ADR_ACK:
	bcnt = 0;
	// Reset the TWI Interupt to wait for a new event.
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;

	/* TWI SLAVE REGISTER WRITES */
    case TWI_SRX_ADR_DATA_ACK:
    case TWI_SRX_GEN_DATA_ACK:
	data = TWDR;
	switch (++bcnt) {
	case 0:
	    break;

	case 1:
	    reg = data;
	    
	    /* For readback registers, prepare data here */
	    if (reg == GSTAT) {
		prep_data = 
		    (rs232_havechar() ? RXA232 : 0) |
#if 0
		    (rs485_havechar() ? RXA485 : 0) |
		    (eeprom_is_ready() ? 0 : EEBUSY) |
#endif
		    (ADCSRA & (1<<ADSC) ? ADCBUSY : 0);
	    }
	    else if (reg == ADCDAT) {
		prep_data = ADCL;
	    }
	    else if (reg == RS232D) {
		if (rs232_havechar())
		    prep_data = rs232_getc();
	    }
	    else if (reg == RS485D) {
#if 0
		if (rs485_havechar())
		    prep_data = rs485_getc();
#endif
	    }
	    else if (reg == EEDATA) {
		#if 0
		prep_data = eeprom_read_byte((unsigned char *)eeaddr++);
#endif
	    }
	    else if (reg == RJ_VERSION) {
		prep_data = pgm_read_byte(pgm_ptr);
	    }
	    break;

	case 2:
	    if (reg >= 1 && reg <= 4) {
		servo_pwm = data;
	    }
	    else if (reg == RS232D) {
		rs232_putc(data);
	    }
	    else if (reg == BPS232) {
		lobyte = data;
	    }
	    else if (reg == RS485D) {
#if 0
		rs485_putc(data);
#endif
	    }
	    else if (reg == BPS485) {
		lobyte = data;
	    }
	    else if (reg == ADCMUX) {
		/* Set mux here, force ADLAR bit to zero, and start */
		/* let user i2c setting to select the volt ref src  */
		ADMUX = (data & 0b11001111);
		ADCSRA |= (1 << ADSC);
	    }
	    else if (reg == EEADDR) {
		eeaddr = data;
	    }
	    else if (reg == EEDATA) {
#if 0
		eeprom_write_byte((unsigned char *)eeaddr, data);
#endif
	    }
	    else if (reg == RJ_VERSION) {
		pgm_ptr = VERSION_STR;
	    }
	    else if ((reg == REBOOT) && (data == BOOTVAL)) {
		TWCR |= (1<<TWINT) | (1<<TWEA);
		wdt_enable(WDTO_30MS);
		while(1) {}; 
	    }
	    break;

	default:
	    if (reg >= SERVO_0 && reg <= SERVO_3) {
		servo_pwm = (data << 8) | servo_pwm;
#if 0
		servo_set(reg - 1, servo_pwm);
#endif
	    }
	    else if (reg == RS232D) {
		rs232_putc(data);
	    }
	    else if (reg == RS485D) {
#if 0
		rs485_putc(data);
#endif
	    }
	    else if (reg == BPS232) {
		rs232_swuart_setbaud(data, lobyte);
	    }
	    else if (reg == BPS485) {
#if 0
		rs485_setbaud((data << 8) | lobyte);
#endif
	    }
	    else if (reg == EEADDR) {
		eeaddr = ((data << 8) & 0x01) | eeaddr;
#if 0
		prep_data = eeprom_read_byte((unsigned char *)eeaddr++);
#endif
	    }
	    break;
	}
	// Reset the TWI Interupt to wait for a new event.
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;

    case TWI_SRX_STOP_RESTART:
	// Enter not addressed mode and listen to address match
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;           

    case TWI_SRX_ADR_DATA_NACK:
    case TWI_SRX_GEN_DATA_NACK:
    case TWI_STX_DATA_ACK_LAST_BYTE:
    case TWI_BUS_ERROR:
	TWCR = (1<<TWSTO) | (1<<TWINT);
	break;

    default:
	TWCR = (1<<TWEN) | (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWWC);
	break;
    }
}
#endif

 