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
 * \file avr/light.c
 * \author David Kelso - david@kelso.id.au
 * \brief Application to control a light
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "light.h"
#include <avr/io.h>

#include <avr/signal.h>
#include <avr/interrupt.h>

#include <avr/eeprom.h> 
#include <stdlib.h> // malloc
#include <string.h> // memcmp

#define CONSUMER 1
#define PRODUCER 2

#define NUM_SERVICES 2

#define BOOL 1
#define STRING 2
#define INT8 3
#define UINT8 4

struct service {
	uint8_t type;
	uint8_t name_length;
	char * name;
	void (*on_exec)(char * args, uint8_t length);
	uint8_t arg_length;
	uint8_t arguments[];
};

struct destination {
	uint8_t index; // Index of the producer service to send to this destination on
	uint8_t address[4]; // IP address of the destination
	uint16_t port; // Port of the destination
	struct service dest_service;
	struct destination * next; // The next destination in the message list
};

void erase(struct destination *);
void load(struct destination *);
void save(struct destination *);
void turn_exec(char *, uint8_t);
void toggle_exec(char *, uint8_t);

struct service 
	turn_service = {CONSUMER, 4, "turn", &turn_exec, 1, {BOOL}},
	toggle_service = {CONSUMER, 6, "toggle", &toggle_exec, 0, {}};

struct service * services[NUM_SERVICES] = {&turn_service, &toggle_service};

void toggle_exec(char * args, uint8_t length) {
	PORTB = ~PORTB;
}

void turn_exec(char * args, uint8_t length) {
	if (!memcmp(args, "true", length))
		PORTB = 0xff;
	else if (!memcmp(args, "false", length))
		PORTB = 0x00;
	else
		log("Bad turn argument");
}



// The list of devices to message on producing a service
struct destination * message_list;
// The temporary list used when programming
struct destination * temp_message_list;


// Whether the device is in programming mode
uint8_t program_mode = FALSE;
// Whether the device's program switch has been pushed
uint8_t program_pushed = FALSE;

/*
 * light_init - initialise the application
 */
void light_init(void) {
	// uint8_t * addr = 0, i, j, num_messages;
	// struct destination * pointer = NULL, * previous = NULL;

	/*
	if (udp_listen(PORT, &light_callback)) {
		log("port in use");
	}
	*/
	udp_listen(PORT, &light_callback);

	// Grab any message list info out of the eeprom
	load(message_list);
}

/*
 * light_callback - process an incoming packet
 */
void light_callback(UDP_HEADER * header_in) {
	uint8_t * message = &data[IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH];
	uint16_t message_size = header_in->length - UDP_HEADER_LENGTH;

	if (compare_ip(header_in->ip_header->source_ip, bcast_ip)) {
        // Deal with broadcast messages
		log("Broadcast message received");

		// Broadcast codes are 10-19. Check the code of this message
		// 10 - Set Programming Mode
		if (!memcmp(message, "10 program(", 11)) {
			if (!memcmp(message + 11, "true);", 6)) {
				// Enable programming mode
				program_mode = TRUE;
				
				// Go back to idle
				return;
			}
			else if (!memcmp(message + 11, "false);", 7)) {
				if (program_pushed) {
					// Clear the current message list
					erase(message_list);

					// Make the temp message list permanant
					message_list = temp_message_list;
					temp_message_list = NULL;

					// Store the message list in EEPROM
					save(message_list);
				}
				else {
					// Clear the temp message list
					erase(temp_message_list);
				}

				// Change out of programming mode
				program_mode = FALSE;
				program_pushed = FALSE;
			}
			else
				log("Bad program mode message");
		}

		// 11 - Service broadcast
		else if (!memcmp(message, "10 program(false);", 18)) {
		}
		else
			log("Bad broadcast message");
	}

	// Otherwise it's addressed directly to this node
	else if (!memcmp(message, "20 ", 3)) {
		// work out the length of the arguments
		uint8_t start = 3, end, i;

		while (message[start] != '(' && start < message_size)
			start++;
		start++;

		end = start;
		while (message[end] != ')' && end < message_size)
			end++;

		// Check we haven't hit the end
		if (start == message_size || end == message_size) {
			log("Incorrectly formatted message");
			return;
		}
		

		// Find the service it's for
		for (i = 0; i < NUM_SERVICES; i++) {
			if (services[i]->type != CONSUMER) continue;

			if (!memcmp(message + 3, services[i]->name, services[i]->name_length)) {
				services[i]->on_exec(data + start, end - start);
				break;
			}
		}
	}
}

SIGNAL(SIG_INTERRUPT0) {
	struct destination * pointer = message_list;
	while (pointer != NULL) {


		pointer = pointer->next;
	}
}

// Starts at the struct pointed to by pointer and frees 
// all subsequent destinations in the list
void erase(struct destination * pointer) {
	struct destination * previous = NULL;

	while (pointer != NULL) {
		previous = pointer;
		pointer = pointer->next;
		free(previous);
	}
}

void load(struct destination * pointer) {
	struct destination * previous = NULL;
	uint8_t num_messages, i, j, * addr = (uint8_t *) 0;

	num_messages = eeprom_read_byte(addr++);
	for (i = 0; i < num_messages; i++) {
		pointer = malloc(sizeof(struct destination));

		pointer->index = eeprom_read_byte(addr++);
		pointer->address[0] = eeprom_read_byte(addr++);
		pointer->address[1] = eeprom_read_byte(addr++);
		pointer->address[2] = eeprom_read_byte(addr++);
		pointer->address[3] = eeprom_read_byte(addr++);
		pointer->port = eeprom_read_byte(addr++) << 8;
		pointer->port += eeprom_read_byte(addr++);

		pointer->dest_service.name_length = eeprom_read_byte(addr++);
		for (j = 0; j < pointer->dest_service.name_length; j++) {
			// This has got to be wrong
			pointer->dest_service.name[j] = eeprom_read_byte(addr++);
		}

		pointer->dest_service.arg_length = eeprom_read_byte(addr++);
		for (j = 0; j < pointer->dest_service.arg_length; j++) {
			// This too
			pointer->dest_service.arguments[j] = eeprom_read_byte(addr++);
		}

		// Tag this on to the end of the linked list
		if (previous == NULL)
			message_list = pointer;
		else
			previous->next = pointer;

		previous = pointer;
	}
}
void save(struct destination * pointer) {
	uint8_t * addr = (uint8_t *) 1;
	uint8_t count = 0;

	while (pointer != NULL) {
		uint8_t j;
		count++;

		eeprom_write_byte(addr++, pointer->index);
		eeprom_write_byte(addr++, pointer->address[0]);
		eeprom_write_byte(addr++, pointer->address[1]);
		eeprom_write_byte(addr++, pointer->address[2]);
		eeprom_write_byte(addr++, pointer->address[3]);
		eeprom_write_byte(addr++, pointer->port >> 8);
		eeprom_write_byte(addr++, pointer->port & 0x00ff);

		eeprom_write_byte(addr++, pointer->dest_service.name_length);
		for (j = 0; j < pointer->dest_service.name_length; j++) {
			// This has got to be wrong
			eeprom_write_byte(addr++, pointer->dest_service.name[j]);
		}

		eeprom_write_byte(addr++, pointer->dest_service.arg_length);
		for (j = 0; j < pointer->dest_service.arg_length; j++) {
			// This too
			eeprom_write_byte(addr++, pointer->dest_service.arguments[j]);
		}

		pointer = pointer->next;
	}
	// Write the count back to the first byte
	eeprom_write_byte(0, count);
}

SIGNAL(SIG_INTERRUPT1) {
	uint8_t i, need_to_send;

	// The program button has been pressed
	
	// If not yet in program mode, get into it
	if (!program_mode) {
		program_mode = TRUE;
	}

	if (program_pushed) {
		// Program has already been pushed - that means we want
		// to end the programming mode

		// Broadcast end of programming
		IPV4_HEADER ip_header;
		UDP_HEADER udp_header;
		uint16_t position = IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH;

		// Fill the UDP header
		udp_header.ip_header = &ip_header;
		udp_header.source_port = PORT;
		udp_header.dest_port = PORT;
		udp_header.length = 123; // FIXME;

		// Fill the IP header
		memcpy(ip_header.source_ip, local_ip, 4);
		memcpy(ip_header.dest_ip, bcast_ip, 4);
		ip_header.protocol = UDP_PROTOCOL;
		ip_header.length = 456; // FIXME;

		// Copy the message prefix into the buffer
		memcpy(data + position, "10 program(false);", 18);

		// Make temp list permanant
		erase(message_list);
		message_list = temp_message_list;
			
		// Store the message list to eeprom
		save(message_list);

		// Change out of program mode
		program_mode = FALSE;
		program_pushed = FALSE;

		// Return to idle mode
		return;
	}

	// Remember that this device has been pushed
	program_pushed = TRUE;

	// Does the device have any consumer services?
	need_to_send = FALSE;
	for (i = 0; i < NUM_SERVICES; i++) {
		if (services[i]->type == CONSUMER) {
			need_to_send = TRUE;
			break;
		}
	}

	if (need_to_send) {
		// Transmit a broadcast with all this device's consumer messages
		IPV4_HEADER ip_header;
		UDP_HEADER udp_header;
		uint16_t position = 0;

		// Fill the UDP header
		udp_header.ip_header = &ip_header;
		udp_header.source_port = PORT;
		udp_header.dest_port = PORT;
		udp_header.length = 123; // FIXME;

		// Fill the IP header
		memcpy(ip_header.source_ip, local_ip, 4);
		memcpy(ip_header.dest_ip, bcast_ip, 4);
		ip_header.protocol = UDP_PROTOCOL;
		ip_header.length = 456; // FIXME;

		// Copy the message prefix into the buffer
		memcpy(data + 789, "11 ", 3);

		for (i = 0; i < NUM_SERVICES; i++) {
			uint8_t j;
			// Only transmit consumer services
			if (services[i]->type != CONSUMER) continue;

			// Put in the name
			memcpy(data + position, 
				services[i]->name, 
				services[i]->name_length);

			position += services[i]->name_length;

			// Put in the arguments
			for (j = 0; j < services[i]->arg_length; j++) {
				if (j == 0) data[position++] = '(';

				// Put in the data type
				if (services[i]->arguments[j] == BOOL) {
					memcpy(data + position, "bool", 4);
					position += 4;
				}
				else if (services[i]->arguments[j] == INT8) {
					memcpy(data + position, "int8", 4);
					position += 4;
				}
				else if (services[i]->arguments[j] == UINT8) {
					memcpy(data + position, "uint8", 5);
					position += 5;
				}
				else {
					log("Non-existant data type");
					return;
				}

				
				// Put appropriate formatting after the argument
				if (j == services[i]->arg_length) {
					data[position++] = ',';
					data[position++] = ' ';
				}
				else {
					data[position++] = ')';
				}
			} // end of arguments for-loop

			// Put a space between messages
			data[position++] = ' ';
		} // end of services for-loop
		ipv4_send(&ip_header);
	}
}
