/*
 * Copyright (c) 2005 David Kelso <david@kelso.id.au>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * Smart Framework - AVR Implementation
 * 
 * \file avr/main.c
 * \author David Kelso - david@kelso.id.au
 * \brief Main program containing initialisation and idle loop
 */
#include <avr/io.h>

#include <avr/eeprom.h>

// Interrupt handling
#include <avr/signal.h>
#include <avr/interrupt.h>

#include "main.h"
#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "slip.h"
#include "service.h"

#if defined echo
#include "echo.h"
#endif

#if defined light
#include "light.h"
#elif defined toggleswitch
#include "toggleswitch.h"
#elif defined polarswitch
#include "polarswitch.h"
#endif

uint8_t counter = 0;

// Initialise the on-chip UART
void uart_init() {
	// UBRR - UART Baud Rate Register
	// Initialise the baud rate controller
	// From datasheet: BAUD = FREQUENCY / 16*(UBRR+1)
	UBRRH = 0;
	UBRRL = F_CPU / 16 / BAUD - 1;

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
	// Bits 2,1,0 - CS12, CS11, CS10: Clock Select1
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
	MCUCR = _BV(ISC11) | _BV(ISC10);
#if defined toggleswitch
	MCUCR |= _BV(ISC01) | _BV(ISC00);
#elif defined polarswitch
	MCUCR |= _BV(ISC01);
#endif

	// General Interrupt Control Register – GICR
	// Bit 6 – INT0: External Interrupt Request 0 Enable
	GICR = _BV(INT1);
#if defined toggleswitch || defined polarswitch
	GICR |= _BV(INT0);
#endif
}

// Main function and idle loop
int main() {
	DDRB = 0xff;
	DDRD = 0xf2; // 111?0010

	// Set PORTD to zero in order to
	//  - disable transmitting to the network
	//  - set the interrupt pull up to low
	PORTD = 0;

	// Disable the analog comparitor
	ACSR = _BV(ACD);
	ADCSRA = 0;

	// Initialise the UART for serial communication
	uart_init();
	
	// Initialise the timer for periodic checks
	// timer_init();

	external_init();

#if defined echo
	echo_init();
#endif

	service_init();

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
	if (counter > 10) {
		PORTB = ~PORTB;
		counter = 0;

	}
	else 
		counter++;
}
