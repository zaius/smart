/**
 * Internet Zero - Unix Gateway
 * 
 * \file unix/slip.c
 * \author David Kelso - david@kelso.id.au
 * \brief SLIP encoder/decoder
 */

#include <sys/types.h>
#include <stdlib.h>
#include "slip.h"

/// The length of the buffers to use in the linked list. This should be set
/// higher than the maximum possible packet size 
#define BUFFER_LENGTH 4000

// Encode an array of data using SLIP encoding
size_t slip_encode(uint8_t ** dest, uint8_t * source, size_t length) {
	int i, j = 0;
	uint8_t c;
	uint8_t * buffer;

	// If someone handed a packet with characters that all needed 
	// escaping, there would be twice as many characters plus a 
	// SLIP_END character at the start and end.
	buffer = malloc((length * 2 + 2) * sizeof(uint8_t));

	// Point the destination pointer to the address of the array
	*dest = buffer;

	buffer[j++] = SLIP_END;

	for (i = 0; i < length; i++) {
		c = source[i];

		if (c == SLIP_END) {
			buffer[j++] = SLIP_ESC;
			buffer[j++] = SLIP_ESC_END;
		}
		else if (c == SLIP_ESC) {
			buffer[j++] = SLIP_ESC;
			buffer[j++] = SLIP_ESC_ESC;
		}
		else {
			buffer[j++] = c;
		}
	}

	buffer[j++] = SLIP_END;

	return j;
}


typedef struct packet PACKET;
struct packet {
	// Pointer to the data for the packet
	uint8_t data[BUFFER_LENGTH];
	size_t length;
	PACKET * next;
}
PACKET * first = NULL, * pointer = NULL;


int slip_add_data(uint8_t * source, size_t length) {
	int source_pos, count = 0;
	static int dest_pos;
	// Since packets are bordered with SLIP_END characters on each side we
	// need to save the data between one set of SLIP_END characters and
	// ignore the data between the other (there should be nothing so it will
	// only be noise
	static int inpacket; 
	uint8_t c, previous;

	for (source_pos = 0; source_pos < length; source_pos++) {
		c = source[source_pos];

		if (c == SLIP_END) { // End (or beginning) of a packet
			if (inpacket) { // End of a packet
				inpacket = FALSE;

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
			c = source[source_pos++];

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
		if (dest_pos < BUFFER_LENGTH) 
			buffer[dest_pos++] = c;
	}

	// We've added all the data, count the number of ready packets
	pointer = first;
	while (pointer->next != null)
		count++;

	// If we're halfway through a packet, it doesn't count as complete
	if (inpacket) return count;
	// If one packet has finished but another hasn't started, a new struct wont
	// have been created on the end of the linked list yet - add one to return
	else return count + 1;
}


// take the first packet off the linked list and return the data
size_t slip_retrieve(uint8_t ** dest) {
	// If there are no packets ready then return -1
	if (first == NULL) return -1;

	PACKET * packet = first;

	// Move the first pointer on
	first = first->next;
	
	dest = &(packet->data);
	// Will this free the data inside?
	// free(packet);
	return packet->length;
}