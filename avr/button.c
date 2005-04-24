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
 * \file avr/button.c
 * \author David Kelso - david@kelso.id.au
 * \brief Application for controlling a standard switch/button
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "button.h"

#include <avr/io.h>
#include <avr/signal.h>
#include <avr/interrupt.h>

#include <string.h>

/*
 * button_init - initialise the application
 */
void button_init(void) {
	/*
	if (udp_listen(PORT, &button_callback)) {
		log("port in use");
	}
	*/
	udp_listen(PORT, &button_callback);
}

/*
 * button_callback - process an incoming packet
 */
void button_callback(UDP_HEADER * header_in) {
}

SIGNAL(SIG_INTERRUPT0) {
	UDP_HEADER udp_header;
	IPV4_HEADER ip_header;

	PORTB = ~PORTB;

	data_length = UDP_HEADER_LENGTH + IPV4_HEADER_LENGTH;
	
	strlcpy(data + data_length, "toggle", 6);
	data_length += 6;
	
	udp_header.source_port = PORT;
	udp_header.remote_port = 1337;
	udp_header.length = data_length - IPV4_HEADER_LENGTH;

	ip_header.source_ip[0] = local_ip[0];
	ip_header.source_ip[1] = local_ip[1];
	ip_header.source_ip[2] = local_ip[2];
	ip_header.source_ip[3] = local_ip[3];

	ip_header.dest_ip[0] = 10;
	ip_header.dest_ip[1] = 0;
	ip_header.dest_ip[2] = 0;
	ip_header.dest_ip[3] = 1;

	ip_header.protocol = UDP_PROTOCOL;
	ip_header.length = data_length;

	udp_header.ip_header = &ip_header;

	udp_send(&udp_header);
}
