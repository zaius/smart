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
 * \file avr/slip.c
 * \author David Kelso - david@kelso.id.au
 * \brief SLIP device driver
 */

#include <avr/io.h> // UART Register access
#include <stdlib.h> // random
#include <avr/delay.h>
#include "conf.h" // Data buffers
#include "slip.h"
#include "ipv4.h"

uint8_t retries = 0;
uint8_t slip_send() {
	uint8_t i, c, collision = FALSE;

	if (slip_putc(SLIP_END)) collision = TRUE;

	for (i = 0; !collision && i < data_length; i++) {
		c = data[i];

		if (c == SLIP_END) {
			if (slip_putc(SLIP_ESC)) collision = TRUE;
			if (slip_putc(SLIP_ESC_END)) collision = TRUE;
		}
		else if (c == SLIP_ESC) {
			if (slip_putc(SLIP_ESC)) collision = TRUE;
			if (slip_putc(SLIP_ESC_ESC)) collision = TRUE;
		}
		else {
			if (slip_putc(c)) collision = TRUE;
		}
	}

	if (!collision)
		if (slip_putc(SLIP_END)) collision = TRUE;

	// If there has been no collision, the packet has been transmitted fine
	if (!collision) return 0;

	retries++;

	// if we've had more than MAX_RETRIES, give up; otherwise wait a random
	// time and run slip_send again
	if (retries > MAX_RETRIES) return -1;
	

	// The maximal possible delay is 262.14 ms / F_CPU in MHz.
	// 262.14 / 3.6864 = 71.11 ms
	msleep(random() * retries);
	return slip_send();
}

void msleep(uint8_t delay) {
	uint8_t i = 0;
	// 4 operations per cycle
	// max 65536 cycles
	// F_CPU / 1000 operations per ms

	for (i = 0; i < delay; i++) 
		_delay_loop_2(F_CPU / 1000 / 4);
}

uint8_t c, previous;
uint16_t length = 0;
int inpacket = FALSE;
int seeded = FALSE;
void slip_poll(void) {
	// Keep looping while we have characters
	while (slip_getc(&c)) {

		// If it hasn't happened yet, seed the random number generator
		if (!seeded) {
			srandom(TCNT0);
			seeded = TRUE;
		}

		if (c == SLIP_END) { // The framing character
			if (inpacket) {
				uint8_t version;

				inpacket = FALSE;

				// End marker found, we have a complete packet
				// Move the length into the global variable
				data_length = length;
				length = 0;

				// Get the version from the packet so we know which network layer
				// to hand it to
				version = data[0] >> 4;

				// TODO: Need to do the whole #ifdef thing and check whether we 
				// have certain versions compiled in
				if (version == 4) {
					ipv4_receive();
					return;
				}

				// There was no handler for the specified version
				log("Invalid version number");
				return;
			}
			else { // Start of a packet
				inpacket = TRUE;
				continue;
			}
		}
		else if (!inpacket) {
			// If we're not in a packet, this will just be noise so skip it
			continue;
		}
		else if (previous == SLIP_ESC) {
			// Previous read byte was an escape byte, so this byte will be
			// interpreted differently from others.
			if (c == SLIP_ESC_END)
				c = SLIP_END;
			if (c == SLIP_ESC_ESC)
				c = SLIP_ESC;
		}
		else if (c == SLIP_ESC) {
			previous = c;
			continue;
		}

		previous = c;

		data[length] = c;
		length++;

		// If we overflow, dump the packet
		if (length > MAX_DATA_SIZE) {
			length = 0;
			return;
		}
	}
	return;
}


uint8_t slip_getc(uint8_t *c) {
	// Check if the receive bit is set in the UART Status Register
	if (bit_is_set(UCSRA, RXC)) {
		// If it is, we need to make the pointer point to the received
		// character and return non-zero
		*c = UDR;

		// Ignore the character if there was a framing error
		if (bit_is_set(UCSRA, FE)) return 0;

		return 1;
	}
	else {
		// If it isn't, return zero
		return 0;
	}
}


uint8_t slip_putc(uint8_t c) {
	uint8_t rec_char, count = 0, received = FALSE;

	// Wait until the buffer is empty
	loop_until_bit_is_set(UCSRA, UDRE);

	// Load the character into the buffer
	UDR = c;

	// Read the character and check they're the same
	while(!received) {
		received = slip_getc(&rec_char);

		// If the device isn't connected this would freeze indefinitely.
		// Therefore we need a timeout to warn if nothing is being received.
		if (count++ > 250) return -2;
	}

	if (rec_char == c) return 0;
	else return -1;
}
