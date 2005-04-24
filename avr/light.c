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
 * \file avr/light.c
 * \author David Kelso - david@kelso.id.au
 * \brief Application to control a light
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "light.h"
#include <avr/io.h>

#include <avr/signal.h>
#include <avr/interrupt.h>

#include <string.h> // memcmp

#define CONSUMER 1
#define PRODUCER 2

#define BOOL 1
#define INT8 2
#define UINT8 3

struct service {
	uint8_t type;
	char name[10];
	uint8_t arguments[];
};

struct service 
	turn_service = {CONSUMER, "turn", {BOOL}}, //{"foo", "bar"}},
	toggle_service = {CONSUMER, "toggle", {}};

struct service * services[2] = {&turn_service, &toggle_service};


struct destination {
	uint8_t index; // Index of the producer service to send to this destination on
	uint8_t address[4]; // IP address of the destination
	uint16_t port; // Port of the destination
	char * name; // Name of the service
	uint8_t args[3]; // Arguments to the service
	struct destination * next; // The next destination in the message list
};

// The list of devices to message on producing a service
struct destination * message_list;


// Whether the device is in programming mode
uint8_t program_mode = FALSE;
// Whether the device's program switch has been pushed
uint8_t program_pushed = FALSE;

/*
 * light_init - initialise the application
 */
void light_init(void) {
	/*
	if (udp_listen(PORT, &light_callback)) {
		log("port in use");
	}
	*/
	udp_listen(PORT, &light_callback);
}

/*
 * light_callback - process an incoming packet
 */
void light_callback(UDP_HEADER * header_in) {
	uint8_t * message = &data[IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH];
	// uint16_t message_size = header_in->length - UDP_HEADER_LENGTH;

	if (header_in->ip_header->source_ip[0] == bcast_ip[0] &&
		header_in->ip_header->source_ip[1] == bcast_ip[1] &&
		header_in->ip_header->source_ip[2] == bcast_ip[2] &&
		header_in->ip_header->source_ip[3] == bcast_ip[3]) 
	{
        // Deal with broadcast messages
		log("Broadcast message received");

		// Broadcast codes are 10-19. Check the code of this message
		// 10 - Set Programming Mode
		if (!memcmp(message, "10 program(", 11)) {
			if (!memcmp(message + 11, "true);", 6)) {
				// Enable programming mode
				program_mode = TRUE;
				// Copy messages offered into a buffer
			}
			else if (!memcmp(message + 11, "false);", 7)) {
				// Disable programming mode
				program_mode = FALSE;
				// Check whether producer or consumer
				// If producer, check whether the program button had been pushed
				// If yes, store all addresses to send messages to
				// Clear compatibility buffer
			}
			else
				log("Bad program mode message");
		}

		// 11 - Service broadcast
		else if (!memcmp(message, "10 program(false);", 18)) {
		}
		else
			log("Bad broadcast message");
	}

	// Otherwise it's addressed directly to this node
	else if (!memcmp(message, "20 ", 3)) {
		if (!memcmp(message + 3, "toggle;", 6)) {
			PORTB = ~PORTB;
		}
		else if (!memcmp(message + 3, "turn(false);", 6)) {
			PORTB = 0x00;
		}
		else if (!memcmp(message + 3, "turn(true);", 6)) {
			PORTB = 0xff;
		}
		else {
			log("No match on service");
		}
	}

	else
		log("Unsupported message");
}

SIGNAL(SIG_INTERRUPT0) {
	// The main interrupt has fired
}

SIGNAL(SIG_INTERRUPT1) {
	// The program button has been pressed
	
	// If not yet in program mode, get into it
	if (!program_mode) {
		program_mode = TRUE;
		// Remember that this device has been pushed
		program_pushed = TRUE;
	}
}
