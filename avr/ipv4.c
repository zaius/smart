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
 * \file avr/ipv4.c
 * \author David Kelso - david@kelso.id.au
 * \brief Internet Protocol (version 4) implementation
 */

#include <avr/io.h> // Carry flag hack

#include "ipv4.h"
#include "conf.h"
#include "udp.h"
#include "icmp.h"
#include "slip.h"

/// The default local IP address
uint8_t local_ip[4] = { 10, 0, 0, 2 };
/// The default broadcast address
uint8_t bcast_ip[4] = { 10, 0, 0, 255 };


uint8_t compare_ip(uint8_t * one, uint8_t * two) {
	if (one[0] != two[0]) return FALSE;
	if (one[1] != two[1]) return FALSE;
	if (one[2] != two[2]) return FALSE;
	if (one[3] != two[3]) return FALSE;

	return TRUE;
}

void ipv4_send(IPV4_HEADER *header) {
	uint8_t position = 0, i; 
	uint16_t checksum = 0;

	// Version and Header Length
	data[position++] = ((VER & 0x0f) << 4) | (IHL & 0x0f);

	// Type of Service
	data[position++] = TOS;

	// Total Length
	data[position++] = header->length >> 8;
	data[position++] = header->length & 0x00ff;

	// Identification
	data[position++] = ID >> 8;
	data[position++] = ID & 0x00ff;

	// 3 bit flags, 5 upper fragment offset bits
	data[position++] = ((FLAGS & 0x07) << 5) | (FRAGMENT_OFFSET >> 11);
	// 8 lower fragment offset bits
	data[position++] = (FRAGMENT_OFFSET & 0x00ff);

	// Time to Live
	data[position++] = TTL;

	// Protocol
	data[position++] = header->protocol;
	
	// Zero the checksum fields for calculation
	data[position++] = 0;
	data[position++] = 0;

	// Source address
	data[position++] = header->source_ip[0];
	data[position++] = header->source_ip[1];
	data[position++] = header->source_ip[2];
	data[position++] = header->source_ip[3];

	// Destination address
	data[position++] = header->dest_ip[0];
	data[position++] = header->dest_ip[1];
	data[position++] = header->dest_ip[2];
	data[position++] = header->dest_ip[3];

	// Header Checksum
	i = position;
	while (i > 1) {
		uint16_t calc;
		calc  = data[--i];
		calc += data[--i] << 8;
		
		checksum += calc;

		// If the carry bit is set we had overflow - add one to the checksum
		if (SREG & 0x01) checksum++;
	}
	checksum = ~checksum;

	// Insert the checksum into the packet
	data[10] = checksum >> 8;
	data[11] = checksum & 0x00ff;

	slip_send();
}

void ipv4_receive() {
	uint8_t position = 0, i;
	uint16_t checksum = 0;
	IPV4_HEADER header;

	// Header Checksum...
	// From RFC1071: To check a checksum, the 1's complement sum is computed 
	// over the same set of octets, including the checksum field.  If the 
	// result is all 1 bits (-0 in 1's complement arithmetic), the check 
	// succeeds.
	i = IPV4_HEADER_LENGTH;
	while (i > 1) {
		// TODO: Maybe we can drop out calc and add straight into the 
		// checksum? Would need to do carries carefully
		uint16_t calc;
		calc  = data[--i];
		calc += data[--i] << 8;
		
		checksum += calc;

		// If the carry bit is set we had overflow - add one to the checksum
		// if (SREG & 0x01) calc_upper++;
		if (SREG & 0x01) checksum++;
	}

	// Drop the packet if the checksum doesn't match
	if (checksum != 0xffff) {
		log("Packet failed IPv4 header checksum");
		return;
	}

	// Version and Header Length
	position++;

	// Type of Service
	position++;

	// Total Length
	header.length  = data[position++] << 8;
	header.length += data[position++];

	// Identification
	position += 2;

	// 3 bit flags, 5 upper fragment offset bits
	position++;
	// 8 lower fragment offset bits
	position++;

	// Time to Live
	position++;

	// Protocol
	header.protocol = data[position++];
	
	// Checksum (Done already)
	position += 2;

	// Source address
	header.source_ip[0] = data[position++];
	header.source_ip[1] = data[position++];
	header.source_ip[2] = data[position++];
	header.source_ip[3] = data[position++];

	// Destination address
	header.dest_ip[0] = data[position++];
	header.dest_ip[1] = data[position++];
	header.dest_ip[2] = data[position++];
	header.dest_ip[3] = data[position++];

	// Drop any packets not addressessed to this node or the broadcast address
	if (!compare_ip(header.dest_ip, local_ip) ||
		!compare_ip(header.dest_ip, bcast_ip))
		return;
	
	// Pass the packet off
	// TODO: Need to do the whole #ifdef thing and check whether we 
	// have certain versions protocols in
	if (header.protocol == UDP_PROTOCOL)
        udp_receive(&header);
	else
		log("No valid transport layer, dropping packet");
}