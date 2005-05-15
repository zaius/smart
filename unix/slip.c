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
 * \file unix/slip.c
 * \author David Kelso - david@kelso.id.au
 * \brief SLIP encoder/decoder
 */

#include <sys/types.h>
#include <stdlib.h>
#include <err.h>
#include <string.h> //memcpy
#include "slip.h"

#include <stdio.h>

#define FALSE 0
#define TRUE 1

/// The length of the buffers to use in the linked list. This should be set
/// higher than the maximum possible packet size 
#define BUFFER_LENGTH 4000

// Encode an array of data using SLIP encoding
size_t slip_encode(uint8_t * dest, size_t dest_size, uint8_t * source, size_t source_size) {
	int i, j = 0;
	uint8_t c;

	// Zero sized buffers can break things
	if (dest_size < 1 || dest == NULL || source == NULL) return 0;

	dest[j] = SLIP_END;

	for (i = 0; i < source_size; i++) {
		c = source[i];

		if (c == SLIP_END) {
			if (++j < dest_size) dest[j] = SLIP_ESC;
			if (++j < dest_size) dest[j] = SLIP_ESC_END;
		}
		else if (c == SLIP_ESC) {
			if (++j < dest_size) dest[j] = SLIP_ESC;
			if (++j < dest_size) dest[j] = SLIP_ESC_ESC;
		}
		else {
			if (++j < dest_size) dest[j] = c;
		}
		
	}

	if (++j < dest_size) dest[j] = SLIP_END;

	return ++j;
}


typedef struct packet PACKET;
struct packet {
	// Pointer to the data for the packet
	uint8_t data[BUFFER_LENGTH];
	size_t length;
	PACKET * next;
};

PACKET * first = NULL, * pointer = NULL;
// Since packets are bordered with SLIP_END characters on each side we
// need to save the data between one set of SLIP_END characters and
// ignore the data between the other (there should be nothing so it will
// only be noise
int dest_pos = 0;
int escaped = FALSE;

int slip_add_data(uint8_t * source, size_t length) {
	int source_pos, count = 0;
	uint8_t c;

	if (first == NULL)
		first = malloc(sizeof(PACKET));
		
	for (source_pos = 0; source_pos < length; source_pos++) {
		c = source[source_pos];

		if (c == SLIP_END) { // End (and beginning) of a packet
			// If there hasn't been any data, just skip the packet
			if (dest_pos == 0)
				continue;
				
			// Get to the last/current packet
			// (First should never be null by here so no need to check)
			pointer = first;
			while (pointer->next != NULL)
				pointer = pointer->next;
				
			// Move the data into the pointer
			pointer->length = dest_pos;

			// Reset the destination length for the next packet
			dest_pos = 0;


			// put this new packet on the end of the linked list
			pointer->next = malloc(sizeof(PACKET));
			pointer = pointer->next;
			pointer->next = NULL;
			
			// We're done with the SLIP_END character, time for some data
			continue;
		}
		
		else if (escaped) {
			if (c == SLIP_ESC_END)
				c = SLIP_END;
			else if (c == SLIP_ESC_ESC)
				c = SLIP_ESC;
			else {
				// SLIP_ESC should only be used to escape ESC_END and ESC_ESC
				// so if we get here, warn the user and just act like we
				// ignored the SLIP_ESC character (recommended by RFC1055)
				warn("SLIP escape character used to escape nothing!");
			}
			escaped = FALSE;
		}
		
		else if (c == SLIP_ESC) {
			// If we receive an escape char, read the next char
			escaped = TRUE;
			continue;
		}
		
		// If we get this far the info is data so add it to the buffer
		if (dest_pos < BUFFER_LENGTH) {
			// Move the pointer to the current packet
			pointer = first;
			while (pointer->next != NULL) 
				pointer = pointer->next;
			pointer->data[dest_pos++] = c;
		}
	}

	// We've added all the data, count the number of ready packets
	pointer = first;
	// Check whether the next one exists - the last packet is always in progress
	while (pointer->next != NULL) {
		pointer = pointer->next;
		count++;
	}

	return count;
}


// take the first packet off the linked list and return the data
size_t slip_retrieve(uint8_t * dest, size_t dest_size) {
	size_t source_size;
	uint8_t * source;
	int length;
	
	// If there are no packets ready then return -1
	if (first == NULL) return -1;
	if (first->next == NULL) return -1;

	// Get pointers to the data from the packet
	source = first->data;
	source_size = first->length;

	// Find the smaller of the buffers
	if (source_size > dest_size)
		length = dest_size;
	else 
		length = source_size;

	// Copy the data
	memcpy(dest, source, length);

	
	// Move the first pointer on
	pointer = first;
	first = first->next;
	free(pointer);

	return source_size;
}
