/**
 * Internet Zero - Unix Gateway
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
	if (dest_size < 1 || source_size < 1) return 0;

	dest[j++] = SLIP_END;

	for (i = 0; i < source_size; i++) {
		// If we've hit the limit on the destination, get out
		// FIXME: I have a feeling there still could be an error with this and the adding of the slip_end character after the loop
		if (j >= dest_size) return -1;
		
		c = source[i];

		if (c == SLIP_END) {
			dest[j++] = SLIP_ESC;
			dest[j++] = SLIP_ESC_END;
		}
		else if (c == SLIP_ESC) {
			dest[j++] = SLIP_ESC;
			dest[j++] = SLIP_ESC_ESC;
		}
		else {
			dest[j++] = c;
		}
		
	}

	dest[j++] = SLIP_END;

	return j;
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
int inpacket = FALSE;
int dest_pos = 0;

int slip_add_data(uint8_t * source, size_t length) {
	int source_pos, count = 0;
	uint8_t c;

	for (source_pos = 0; source_pos < length; source_pos++) {
		c = source[source_pos];

		if (c == SLIP_END) { // End (or beginning) of a packet
			if (inpacket) { // End of a packet
				inpacket = FALSE;
				
				// First should never be null if we're in a packet (CHECK THIS)
				pointer = first;
				while (pointer->next != NULL)
					pointer = pointer->next;
				
				pointer->length = dest_pos;
				dest_pos = 0;
			}
			else { // beginning of a packet
				inpacket = TRUE;

				// put this new packet on the end of the linked list
				PACKET * packet = malloc(sizeof(PACKET));
				if (first == NULL) 
					// There are no packets, so this is the first
					first = packet;
				else {
					// Move the pointer to the end
					pointer = first;
					while (pointer->next != NULL) 
						pointer = pointer->next;

					pointer->next = packet;
				}
			}
			// We're done with the SLIP_END character, time for some data
			continue;
		}
		
		else if (!inpacket) {
			// If we're not in a packet, this will just be noise so skip it
			continue;
		}

		else if (c == SLIP_ESC) {
			// If we receive an escape char, read the next char
			c = source[++source_pos];

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
		}
		
		// If we get this far the info is data so add it to the buffer
		if (dest_pos < BUFFER_LENGTH) {
			// Move the pointer to the current packet
			// FIXME: I really don't know why it crashes when i don't do this
			pointer = first;
			while (pointer->next != NULL) 
				pointer = pointer->next;
			pointer->data[dest_pos++] = c;
		}
	}

	// We've added all the data, count the number of ready packets
	if (first == NULL) return 0;

	pointer = first;
	while (pointer->next != NULL) {
		pointer = pointer->next;
		count++;
	}

	// If we're halfway through a packet, it doesn't count as complete
	if (inpacket) return count;
	// If one packet has finished but another hasn't started, a new struct wont
	// have been created on the end of the linked list yet - add one to return
	else return count + 1;
}


// take the first packet off the linked list and return the data
size_t slip_retrieve(uint8_t * dest, size_t dest_size) {
	size_t source_size;
	uint8_t * source;
	int length;
	
	// If there are no packets ready then return -1
	if (first == NULL) return -1;
	if (first->next == NULL && inpacket) return -1;

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
