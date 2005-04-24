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
 * \file avr/echo.c
 * \author David Kelso - david@kelso.id.au
 * \brief Simple UDP echo application
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "echo.h"
#include <avr/io.h>

// For malloc and free
#include <stdlib.h>

/*
 * echo_init - initialise the application
 */
void echo_init(void) {
	/*
	if (udp_listen(PORT, &echo_callback)) {
		log("port in use");
	}
	*/
	udp_listen(PORT, &echo_callback);
}

/*
 * echo_callback - process an incoming packet
 */
void echo_callback(UDP_HEADER * header_in) {
	UDP_HEADER header_out;
	IPV4_HEADER ip_header_out;

	PORTB = ~PORTB;

	header_out.ip_header = &ip_header_out;

	// Reset the length
	data_length = header_in->ip_header->length;
	
	header_out.source_port = PORT;
	header_out.remote_port = header_in->source_port;
	header_out.length = header_in->length;

	header_out.ip_header->protocol = UDP_PROTOCOL;
	header_out.ip_header->length = data_length;

	// header_out.ip_header->source_ip = local_ip;
	header_out.ip_header->source_ip[0] = local_ip[0];
	header_out.ip_header->source_ip[1] = local_ip[1];
	header_out.ip_header->source_ip[2] = local_ip[2];
	header_out.ip_header->source_ip[3] = local_ip[3];

	header_out.ip_header->dest_ip[0] = header_in->ip_header->source_ip[0];
	header_out.ip_header->dest_ip[1] = header_in->ip_header->source_ip[1];
	header_out.ip_header->dest_ip[2] = header_in->ip_header->source_ip[2];
	header_out.ip_header->dest_ip[3] = header_in->ip_header->source_ip[3];

	udp_send(&header_out);
}
