/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/udp.c
 * \author David Kelso - david@kelso.id.au
 * \brief User Datagram Protocol transport layer implementation
 *
 * ftp://ftp.rfc-editor.org/in-notes/rfc768.txt
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"

// For malloc and free
#include <stdlib.h>

// Application callbacks are held in a singularly linked list of node structs
typedef struct node NODE;
struct node {
	uint16_t port; 
	void (*callback)(struct udp_header*);
	NODE * next;
};

// A pointer to the first node in the linked list
NODE * first = NULL;


// Method used for passing packet down to ip for sending
void udp_send(UDP_HEADER * header) {
	uint8_t position = 0;

	// Source port
	data[position++] = header->source_port >> 8;
	data[position++] = header->source_port & 0x00ff;
	
	// Destination port
	data[position++] = header->remote_port >> 8;
	data[position++] = header->remote_port & 0x00ff;

	// Length
	data[position++] = header->length >> 8;
	data[position++] = header->length & 0x00ff;

	// Checksum - zero
	data[position++] = 0;
	data[position++] = 0;

	// Pass down to ipv4
	ipv4_send(header->ip_header);
}


// Method used for passing the packet up from the network layer
void udp_receive(struct ipv4_header * header_in) {
	uint8_t position = 0;
	uint16_t checksum;

	UDP_HEADER * header_out = malloc(sizeof(UDP_HEADER));
	header_out->ip_header = header_in;

	// Source port
	header_out->source_port  = data[position++] << 8;
	header_out->source_port += data[position++];
	
	// Destination port
	header_out->remote_port  = data[position++] << 8;
	header_out->remote_port += data[position++];

	// Length
	header_out->remote_port  = data[position++] << 8;
	header_out->remote_port  = data[position++];
	
	// If there's a checksum, check it
	checksum  = data[position++] << 8;
	checksum += data[position++];

	if (checksum != 0) {
		// TODO: Check it
	}

	// Pass up to application
	NODE * pointer = first;
	while (pointer != NULL) {
		if (pointer->port == header_out->remote_port) {
			// We've found an application listening on this port
			pointer->callback(header_out);

			free(header_out);
			return;	
		}
		pointer = pointer->next;
	}

	// No application is listening on that port, dump the packet
	log("No application listening on that port");

	free(header_out);
	return;
}


// Register an application with udp 
// TODO: error code return values
void udp_listen(uint16_t port, void (*callback)(UDP_HEADER *udp_header)) {
	NODE * pointer = first;
	if (pointer == NULL) {
		// This is the first application
		pointer = malloc(sizeof(NODE));
	}
	else {
		// Move to the last item
		// TODO: Check whether port is in use
		while (pointer->next != NULL) pointer = pointer->next;

		pointer->next = malloc(sizeof(NODE));
		pointer = pointer->next;
	}
	
	pointer->port = port;
	pointer->callback = callback;
	pointer->next = NULL;
}


// Stop an application listening on a port
// TODO: error code return values
void udp_forget(uint16_t port) {
	NODE * pointer = first;
	NODE * previous = NULL;

	while (pointer != NULL) {
		if (pointer->port == port) {
			// We've found an application listening on this port, drop it
			// Take the entry out of the list
			if (previous == NULL)
				first = pointer->next;
			else
				previous->next = pointer->next;
			
			// Free the memory for the item
			free(pointer);
			return;	
		}

		previous = pointer;
		pointer = pointer->next;
	}

	log("No app listening on that port");
	return;
}
