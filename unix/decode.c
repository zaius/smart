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
 * Smart Framework - Unix Gateway
 *
 * \file unix/decode.c
 * \author David Kelso - david@kelso.id.au
 * \brief Human readable IP packet decoder
 */

#include <stdio.h>
#include <inttypes.h>

void decode(uint8_t * data, size_t length) {
	uint8_t position = 0, i;
	uint32_t calc = 0;
	uint16_t checksum = 0;
	int temp;

	// Header Checksum...
	// From RFC1071: To check a checksum, the 1's complement sum is computed 
	// over the same set of octets, including the checksum field.  If the 
	// result is all 1 bits (-0 in 1's complement arithmetic), the check 
	// succeeds.
	printf("Length: %d\n", length);
	if (length < 20) {
		printf("Not long enough to be a full IP packet\n");
		return;
	}

	for (i = 0; i < 20; i += 2) { //while (i > 1) {
		calc += (data[i] << 8) + data[i+1];
	}

	while (calc >> 16) 
		calc = (calc & 0xffff) + (calc >> 16);

	checksum = (calc & 0xffff);

	// Drop the packet if the checksum doesn't match
	if (checksum  != 0xffff) {
		printf("Packet failed IPv4 header checksum\n");
		printf("Calculated: %x\n", checksum);
	}
	else {
		printf("Checksum OK\n");
	}

	// Version and Header Length
	printf("version: %d\n", data[position] >> 4);
	printf("header length: %d\n", data[position] & 0xf0);
	position++;

	// Type of Service
	printf("Type of Service: %d\n", data[position++]);

	// Total Length
	temp = data[position++] << 8;
	temp += data[position++];
	printf("Total Length: %d\n", temp);

	// Identification
	position += 2;

	// 3 bit flags, 5 upper fragment offset bits
	position++;
	// 8 lower fragment offset bits
	position++;

	// Time to Live
	position++;

	// Protocol
	printf("Protocol: %02x\n", data[position++]);

	// Checksum
	temp = data[position++] << 8;
	temp += data[position++];
	printf("Checksum: %02x\n", temp);

	// Source address
	printf("Source Address: %d.%d.%d.%d\n", data[position], data[position+1], data[position+2], data[position+3]);
	position += 4;
	
	// Destination address
	printf("Destination Address: %d.%d.%d.%d\n", data[position], data[position+1], data[position+2], data[position+3]);
	position += 4;

	// Source Port
	temp = data[position++] << 8;
	temp += data[position++];
	printf("UDP Source Port: %d\n", temp);

	// Dest Port
	temp = data[position++] << 8;
	temp += data[position++];
	printf("UDP Destination Port: %d\n", temp);
	
	// Length
	temp = data[position++] << 8;
	temp += data[position++];
	printf("UDP Length: %d\n", temp);

	// Checksum
	position += 2;
	
	// Data
	printf("Data: ");
	while (position < length) 
		printf("%c", data[position++]);
	printf("\n");
}
