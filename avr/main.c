/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/main.c
 * \author David Kelso - david@kelso.id.au
 * \brief Main program containing initialisation and idle loop
 */
#include <avr/io.h>

#include <avr/eeprom.h>
#include <stdlib.h>

// Interrupt handling
#include <avr/signal.h>
#include <avr/interrupt.h>

// #include <inttypes.h>

#include "main.h"
#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "slip.h"
#include "app.h"

struct ipv4_header * ip;
struct udp_header * udp;
// TODO: Fix this - use malloc
struct ipv4_header one;
struct udp_header two;

uint8_t counter;

// Initialise the on-chip UART
void uart_init() {
	// UBRR - UART Baud Rate Register
	// Initialise the baud rate controller
	// From datasheet: BAUD = FREQUENCY / 16*(UBRR+1)
	UBRRH = 0;
	UBRRL = FREQUENCY / 16 / BAUD - 1;

	// UCR - UART Control Register. Enable:
	// Bit 4 - RXEN: Receiver enable
	// Bit 3 - TXEN: Transmitter enable
	UCSRB = _BV(RXEN) | _BV(TXEN);

	// Set the size to be 8 bits
	UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
}

// Initialise 8 bit timer 0
void timer_init() {
	// TIMSK - Timer/Counter Interrupt Mask Register
	// Bit 1 - TOIE0: Timer/Counter0 Overflow Interrupt Enable
	TIMSK = _BV(TOIE0);

	// TCCR0 - Timer/Counter0 Control Register
	// Bits 2,1,0 û CS12, CS11, CS10: Clock Select1, Bits 2, 1 and 0
	// Setting CS12 and CS10 gives us a prescaler of 1024 clock cycles
	TCCR0 = _BV(CS02) | _BV(CS00);
}

// Initialise the external interrupt
void external_init() {
	// MCU Control Register – MCUCR
	// Bit 1, 0 – ISC01, ISC00: Interrupt Sense Control 0 Bit 1 and Bit 0
	// ISC01	ISC00	Description
	// 0		0		The low level of INT0 generates an interrupt request.
	// 0		1		Any logical change on INT0 generates an interrupt request.
	// 1		0		The falling edge of INT0 generates an interrupt request.
	// 1		1		The rising edge of INT0 generates an interrupt request.
	MCUCR = _BV(ISC01) | _BV(ISC00);
	
	// General Interrupt Control Register – GICR
	// Bit 6 – INT0: External Interrupt Request 0 Enable
	GICR = _BV(INT0);
}

// Main function and idle loop
int main() {
	ip = &one;
	udp = &two;
	// ip = malloc(sizeof(ipv4_header));
	// udp = malloc(sizeof(udp_header));

	DDRB = 0xff;

	// Initialise the UART for serial communication
	uart_init();
	
	app_init();

	// Initialise the timer for periodic checks
	timer_init();

	external_init();

	// Set the local address
	ip->local_ip[0] = 192;
	ip->local_ip[1] = 168;
	ip->local_ip[2] = 68;
	ip->local_ip[3] = 2;

	// Set the destination address
	ip->remote_ip[0] = 192;
	ip->remote_ip[1] = 168;
	ip->remote_ip[2] = 68;
	ip->remote_ip[3] = 1;

	ip->protocol = UDP_PROTOCOL;

	udp->source_port = 80;
	udp->remote_port = 1234;
	

	// Seed the random number
	// srand(some counter value?);

	// Enable Interrupts
	sei();

	// Loop forever
	while (TRUE) {
		slip_poll();
	}
}

// Overflow interrupt for timer zero - Used for executing periodic statements
SIGNAL(SIG_OVERFLOW0) {
	// Send every 15 overflows - approx equals 1 second
	if (counter > 15) {
		/* Old method
		uint8_t position = 0;

		data_length = 15;

		uint8_t message[data_length];
		sprintf(message, "rand: %i\n\r", rand());

		ip->length = data_length + IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH;
		udp->length = data_length + UDP_HEADER_LENGTH;

		position = add_ipv4_header(ip, buffer, position);
		position = add_udp_header(udp, buffer, position);

		position = str_cpy(message, data_length, buffer, position);

 		slip_send(buffer, position);


		counter = 0;
		PORTB = ~PORTB;
		*/
	}
	else 
		counter++;
}

SIGNAL(SIG_INTERRUPT0) {
	// eeprom_write_byte (uint8_t *addr, uint8_t val);
	// Timer/Counter Register – TCNT0
	srand(TCNT0);
}

uint8_t str_cpy(char * source, uint8_t length, char * destination, uint8_t position) {
	uint8_t i = 0;
	while (i < length) {
		destination[position++] = source[i++];
	}

	return position;
}
