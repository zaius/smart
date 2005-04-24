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
 * \file avr/ipv6.h
 * \author David Kelso - david@kelso.id.au
 * \brief Internet Protocol (version 6) header
 */

/*
Name			Len 		Description/Notes
version			4 bits 		value = 6. Same location as IPv4 - everything after this changes.
traffic class 	8 bits 		-
Flow Label 		20 bits 	-
payload length 	16 bits 	unsigned length in octets of payload (excludes header but includes extensions)
next header 	8 bits 		Protocol in following header - same values as IPv4.
hop limit 		8 bits 		Maximum number of hops. Same as TTL for IPv4.
source IP 		128 bits 	-
destination IP 	128 bits 	-
*/

#define VER 6
#define CLASS 0
#define FLOW 0
#define HOPS 255

struct ipv6_header {
	uint8_t local_ip[16];
	uint8_t remote_ip[16];
	uint8_t protocol;
	uint16_t length;
};

uint8_t add_ipv6_header(struct ipv6_header * header, uint8_t * buffer, uint8_t position);
