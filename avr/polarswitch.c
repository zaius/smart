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
 * \file avr/polarswitch.c
 * \author David Kelso - david@kelso.id.au
 * \brief Functions to control a switch/button
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "polarswitch.h"
#include "service.h"

#include <avr/signal.h>
#include <avr/interrupt.h>
#include <string.h>        // memcpy

uint8_t name[4] = "turn";
uint8_t args[1] = {BOOL};

struct service
	turn_service = {PRODUCER, 4, name, NULL, 1, args};

struct service * services[NUM_SERVICES] = {&turn_service};


SIGNAL(SIG_INTERRUPT0) {
	struct destination * pointer = message_list;
	uint8_t value = FALSE;

	// Debounce the switch used to call the interrupt (see service.c)
	value = debounce(&PIND, PD2);

	while (pointer != NULL) {
		if (pointer->source_service == &turn_service) {
			// Broadcast end of programming
			IPV4_HEADER ip_header;
			UDP_HEADER udp_header;
			uint16_t position = IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH;

			// Copy the message into the buffer
			memcpy(data + position, "20 turn", 7);
			position += 7;

			if (value) {
				memcpy(data + position, "(true);", 7);
				position += 7;
			}
			else {
				memcpy(data + position, "(false);", 8);
				position += 8;
			}

			data_length = position;

			// Fill the UDP header
			udp_header.ip_header = &ip_header;
			udp_header.source_port = PORT;
			udp_header.dest_port = pointer->port;
			udp_header.length = position - IPV4_HEADER_LENGTH;

			// Fill the IP header
			memcpy(ip_header.source_ip, local_ip, 4);
			memcpy(ip_header.dest_ip, pointer->address, 4);
			ip_header.protocol = UDP_PROTOCOL;
			ip_header.length = position;

			// Send it!
			udp_send(&udp_header);
		}

		pointer = pointer->next;
	}

	GIFR = _BV(INTF0);
}
