/* Internet Zero - AVR Implementation
* David Kelso - david@kelso.id.au
*
* slip.c - Slip device driver
*
*/
#include "conf.h"

#include "slip.h"
#include "ipv4.h"
#include <avr/io.h>
#include <avr/delay.h>

/*
* Send a packet over the slip device
* Blocks until the entire packet is finished
*/
void slip_send() {
	uint8_t i, c;

	slip_putc(SLIP_END);

	for (i = 0; i < data_length; i++) {
		c = data[i];

		if (c == SLIP_END) {
			slip_putc(SLIP_ESC);
			slip_putc(SLIP_ESC_END);
		}
		else if (c == SLIP_ESC) {
			slip_putc(SLIP_ESC);
			slip_putc(SLIP_ESC_ESC);
		}
		else {
			slip_putc(c);
		}
	}

	slip_putc(SLIP_END);
}


/*
 * Poll for a packet on the slip device. If there is a packet available, pass
 * it up to the corresponding network layer.
 */
void slip_poll(void) {
	uint8_t c, previous;
	uint16_t length = 0;

	// Keep looping while we have characters
	while (slip_getc(&c)) {
		if (c == SLIP_ESC) {
			previous = c;
			continue;
		}
		if (c == SLIP_END) {
			// End marker found, we have a complete packet
			// Move the length into the global variable
			data_length = length;

			// Get the version from the packet so we know which network layer
			// to hand it to
			uint8_t version = data[0] >> 4;

			// TODO: Need to do the whole #ifdef thing and check whether we 
			// have certain versions compiled in
			if (version == 4) {
				ipv4_receive();
				return;
			}

			log("Invalid version number");
			return;
		}

		// Uh.. i think this is in the wrong place.. how was it working before?
		// previous = c;

		if (previous == SLIP_ESC) {
			// Previous read byte was an escape byte, so this byte will be
			// interpreted differently from others.
			if (c == SLIP_ESC_END)
				c = SLIP_END;
			if (c == SLIP_ESC_ESC)
				c = SLIP_ESC;
		} 

		previous = c;

		data[length] = c;
		length++;

		// If we overflow, dump the packet
		if (length > MAX_DATA_SIZE) {
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

void slip_putc(uint8_t c) {
	// Wait until the buffer is empty
	loop_until_bit_is_set(UCSRA, UDRE);

	// Load the character into the buffer
	UDR = c;
}
