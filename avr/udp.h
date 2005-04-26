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
 * \file avr/udp.h
 * \author David Kelso - david@kelso.id.au
 * \brief User Datagram Protocol transport layer header file
 *
 * ftp://ftp.rfc-editor.org/in-notes/rfc768.txt
 */

#include <inttypes.h>

/// The protocol number to use in the IP header
#define UDP_PROTOCOL 17
/// The maximum number of applications that can be registered with udp
#define UDP_MAX_APPS 10
#define UDP_HEADER_LENGTH 8

typedef struct udp_header UDP_HEADER;
struct udp_header {
	IPV4_HEADER * ip_header;
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t length;
};

// Prototypes

/**
 * udp_send - 
 * Method used for passing packet down to ip for sending
 * \param header a pointer to the udp header from the packet
 */
void udp_send(UDP_HEADER * header);

/**
 * udp_receive - 
 * Method used for passing the packet up from the network layer
 * \param header a pointer to the ipv4 header from the received packet
 */
void udp_receive(IPV4_HEADER * header);

/**
 * udp_listen -
 * Method used for application to register with udp 
 * \param port the port the application wants to listen on
 * \param callback a pointer to the function to call when a packet 
 * arrives addressed to the specified port
 */
void udp_listen(
	uint16_t port,
	void (*callback)(UDP_HEADER * header));

/**
 * udp_forget - 
 * Method used for application to deregister
 * \param port the port of the application to be deregistered
 */
void udp_forget(uint16_t port);
