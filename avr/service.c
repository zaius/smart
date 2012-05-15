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
* \file avr/service.c
* \author David Kelso - david@kelso.id.au
* \brief Application to coordinate services of smart devices
*/

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "service.h"

#if defined light
#include "light.h"
#elif defined toggleswitch
#include "toggleswitch.h"
#elif defined polarswitch
#include "polarswitch.h"
#endif

#include "slip.h" //msleep

#include <avr/io.h>

#include <avr/signal.h>
#include <avr/interrupt.h>

// #include <avr/eeprom.h>
#include <stdlib.h> // malloc
#include <string.h> // memcmp

/*
* The device stores a list of all the devices (and their corresponding
* services) that need to be informed when a production service fires.
* This destination pointer points to the first in a list of all the
* destinations.
*/
struct destination * message_list;
/* Because the device needs to remember all the service broadcasts it hears
* on the network (so it can later transmit to them), the device needs
* somewhere to store them. This could be the message_list however if a
* device doesn't have it's programming button pressed before the end of
* programming mode it needs to keep its original list. Therefore we can
* use a temporary list instead and free the relevant one when the time comes.
*/
struct destination * temp_message_list;


// Whether the device is in programming mode. If yes, it needs to track all
// the service broadcasts that get sent to it.
volatile uint8_t program_mode;

// Whether the device's program switch has been pushed. If yes it needs to
// lock down all services it receives.
volatile uint8_t program_pushed;

/*
* service_init - initialise the application
*/
void service_init(void) {
	program_mode = FALSE;
	program_pushed = FALSE;
	message_list = NULL;
	temp_message_list = NULL;

	udp_listen(PORT, &service_callback);

	// Grab any message list info out of the eeprom
	//load(message_list);
}

/*
* service_callback - process an incoming packet
*/
void service_callback(UDP_HEADER * header_in) {
	uint8_t * message = data + IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH;
	uint16_t message_size = header_in->length - UDP_HEADER_LENGTH;

	if (compare_ip(header_in->ip_header->dest_ip, bcast_ip)) {
		// Deal with broadcast messages
		log("Broadcast message received");

		// Broadcast codes are 10-19. Check the code of this message
		// 10 - Set Programming Mode
		if (!memcmp(message, "10 program", 10)) {
			if (!memcmp(message + 10, "(true);", 7)) {

				// Enable programming mode
				while (program_mode != TRUE)
					program_mode = TRUE;

				PORTB = program_mode;

				// Go back to idle
				return;
			}
			else if (!memcmp(message + 10, "(false);", 8)) {
				if (program_pushed) {
					// Clear the current message list
					erase(message_list);

					// Make the temp message list permanant
					message_list = temp_message_list;
					temp_message_list = NULL;

					// Store the message list in EEPROM
					// save(message_list);
				}
				else {
					// Clear the temp message list
					erase(temp_message_list);
					temp_message_list = NULL;
				}

				// Change out of programming mode
				program_mode = FALSE;
				program_pushed = FALSE;
			}
			else
				log("Bad program mode message");
		} // end if 10 program(<boolean>);

		// 11 - Service broadcast
		else if (!memcmp(message, "11 ", 3)) {
			uint8_t i;
			for (i = 0; i < NUM_SERVICES; i++) {
				uint8_t position = 3;
				if (services[i]->type != PRODUCER) continue;

				while (TRUE) {
					// Compare service names
					if (!memcmp(message + position, services[i]->name, services[i]->name_length)) {
						// We've had a match! Add it to the temp message list
						struct destination * pointer;

						pointer = malloc(sizeof(struct destination));
						memcpy(pointer->address, header_in->ip_header->source_ip, 4);
						pointer->port = header_in->source_port;
						pointer->source_service = services[i];
						// pointer->dest_service.type = CONSUMER;
						// pointer->dest_service.name_length = services[i]->name_length;
						// pointer->dest_service.name = services[i]->name;
						// FIXME
						// pointer->dest_service.arg_length = 0;
						// pointer->dest_service.arguments = {};
						pointer->next = NULL;

						if (temp_message_list == NULL)
							// There's nothing in the list, make it the first
							temp_message_list = pointer;
						else {
							// Move to the end of the list and put it in
							struct destination * list = temp_message_list;
							while (list->next != NULL)
								list = list->next;

							list->next = pointer;
						}
					}

					// TODO: Compare arguments as well

					// Skip to the next message
					while (message[position] != ')' && position < header_in->length)
						position++;

					// If something wierd has happened and we have reached the end without
					// finding termination, just stop.
					if (position >= header_in->length) break;

					// If the next character is a semicolon that indicated it's the last
					// message of the broadcast.
					if (data[++position] == ';') break;

					// Otherwise there's still messages left. Move to the first char.
					position++;
				}
			}
			return;
		} // end else if 11 <Service Broadcast>
		else
			log("Bad broadcast message");
	}

	// Otherwise it's addressed directly to this node
	// 20 - Service request
	else if (!memcmp(message, "20 ", 3)) {
		// work out the length of the arguments
		uint8_t start = 3, end, i;

		// Make start the first opening bracket
		while (message[start] != '(' && start < message_size)
			start++;

		// Currently start is on the bracket, move it forward
		// to the first character of the arguments
		start++;

		// Make end the first character of the arguments
		end = start;

		// Make end the first closing bracket
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
				services[i]->on_exec(message + start, end - start);
				return;
			}
		}

		// Silently ignore if a message doesn't match
		// TODO: Maybe need to log this

	} // End else if 20 <Service Request>
	else {
		log("Unknown message prefix");
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

/*
void load(struct destination * pointer) {
	struct destination * previous = NULL;
	uint8_t num_messages, i, j;
	uint8_t * addr = (uint8_t *) 0;

	num_messages = eeprom_read_byte(addr++);

	// Because eeprom has the value 255 when erased, we can run
	// into big problems if this doesn't get done
	if (num_messages == 255) {
		log("No data in EEPROM");
		return;
	}

	for (i = 0; i < num_messages; i++) {
		uint8_t * name, length;
		pointer = malloc(sizeof(struct destination));

		// Store the ip address and udp port
		pointer->address[0] = eeprom_read_byte(addr++);
		pointer->address[1] = eeprom_read_byte(addr++);
		pointer->address[2] = eeprom_read_byte(addr++);
		pointer->address[3] = eeprom_read_byte(addr++);
		pointer->port = eeprom_read_byte(addr++) << 8;
		pointer->port += eeprom_read_byte(addr++);

		// The name was all that was stored - look up the service for the rest
		length = eeprom_read_byte(addr++);
		name = malloc(sizeof(uint8_t) * length);
		for (j = 0; j < length; j++) {
			name[j] = eeprom_read_byte(addr++);
		}
		for (j = 0; j < NUM_SERVICES; j++) {
			if (!memcmp(name, services[j], length)) {
				pointer->source_service = services[j];
				break;
			}
		}
		// Maybe should check that we weren't given a dud service from memory
		// or the local services haven't changed... not much can be done
		// about it though.


		pointer->dest_service.name_length = eeprom_read_byte(addr++);
		pointer->dest_service.name = malloc(pointer->dest_service.name_length * sizeof(uint8_t));
		for (j = 0; j < pointer->dest_service.name_length; j++) {
			// This has got to be wrong
			pointer->dest_service.name[j] = eeprom_read_byte(addr++);
		}

		pointer->dest_service.arg_length = eeprom_read_byte(addr++);
		pointer->dest_service.arguments = malloc(pointer->dest_service.arg_length * sizeof(uint8_t));
		for (j = 0; j < pointer->dest_service.arg_length; j++) {
			// This too
			pointer->dest_service.arguments[j] = eeprom_read_byte(addr++);
		}

		// Tag this on to the end of the linked list
		if (previous == NULL)
			message_list = pointer;
		else
			previous->next = pointer;

		pointer->next = NULL;
		previous = pointer;
	}
}
*/

/*
void save(struct destination * pointer) {
	uint8_t * addr = (uint8_t *) 1;
	uint8_t count = 0;

	while (pointer != NULL) {
		uint8_t j;
		count++;

		eeprom_write_byte(addr++, pointer->address[0]);
		eeprom_write_byte(addr++, pointer->address[1]);
		eeprom_write_byte(addr++, pointer->address[2]);
		eeprom_write_byte(addr++, pointer->address[3]);
		eeprom_write_byte(addr++, pointer->port >> 8);
		eeprom_write_byte(addr++, pointer->port & 0x00ff);

		// Write the source service's name to eeprom - name is all we need to
		// reliably find it again.
		eeprom_write_byte(addr++, pointer->source_service->name_length);
		for (j = 0; j < pointer->source_service->name_length; j++) {
			eeprom_write_byte(addr++, pointer->source_service->name[j]);
		}

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
*/

// Be sure to call with a pointer to an input port such as &PINB or similar
uint8_t debounce(volatile uint8_t *port, uint8_t pin) {
	uint8_t value, previous = -1;

	// Get the initial value of the pin
	value = *port & _BV(pin);

	// Loop until the value present on the pin and the value
	// that was there 10 milliseconds ago are equal
	while (previous != value) {
		msleep(20);

		previous = value;
		value = *port & _BV(pin);
	}

	// Bitshift so the returned value is either 0 or 1
	return (value >> pin);
}


SIGNAL(SIG_INTERRUPT1) {
	uint8_t i, need_to_send;
	// The program button has been pressed

	// Debounce the switch used to call the interrupt
	// (Don't use INT1 as the pin - that is an index in a register)
	// If the switch was actually turning off and the interrupt was just
	// caused by bounces (or noise) then ignore the interrupt
	if (debounce(&PIND, PD3) != 1) {
		// Clear any int1 interrupts that might have happened since
		GIFR = _BV(INTF1);
		return;
	}

	if (program_mode) {
		if (program_pushed) {
			// Program had already been pushed - that means we want
			// to end the programming mode

			// Broadcast end of programming
			broadcast("10 program(false);", 18);

			// Make temp list permanant
			erase(message_list);
			message_list = temp_message_list;
			temp_message_list = NULL;

			// Store the message list to eeprom
			// save(message_list);

			// Change out of program mode
			program_mode = FALSE;
			program_pushed = FALSE;

			// Clear any int1 interrupts that might have happened since
			GIFR = _BV(INTF1);

			// Return to idle mode
			return;
		}

		// Else if not pushed, fall through
	}
	else {
		// Send start programming broadcast
		broadcast("10 program(true);", 17);

		// Change to program mode
		program_mode = TRUE;
	}

	// There are two ways to end up here
	// 1. The device is in program mode and the program button
	// hadn't been pushed before
	// 2. The device wasn't in program mode and initiated
	// program mode on the network

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
		uint16_t position = IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH;

		// Copy the message prefix into the buffer
		memcpy(data + position, "11 ", 3);
		position += 3;

		for (i = 0; i < NUM_SERVICES; i++) {
			uint8_t j;

			// Put a space between messages
			if (i != 0) data[position++] = ' ';

			// Only transmit consumer services
			if (services[i]->type != CONSUMER) continue;

			// Put in the name
			memcpy(data + position,
				services[i]->name,
				services[i]->name_length);

			position += services[i]->name_length;

			// Put in the arguments
			data[position++] = '(';
			for (j = 0; j < services[i]->arg_length; j++) {
				// Put appropriate formatting after the previous argument
				if (j != 0) {
					data[position++] = ',';
					data[position++] = ' ';
				}

				// Put in the data type
				if (services[i]->arguments[j] == BOOL) {
					memcpy(data + position, "bool", 4);
					position += 4;
				}
				else if (services[i]->arguments[j] == STRING) {
					memcpy(data + position, "string", 6);
					position += 6;
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
				}

			} // end of arguments for-loop
			data[position++] = ')';

		} // end of services for-loop
		data[position++] = ';';

		// Fill the UDP header
		udp_header.ip_header = &ip_header;
		udp_header.source_port = PORT;
		udp_header.dest_port = PORT;
		udp_header.length = position - IPV4_HEADER_LENGTH;

		// Fill the IP header
		memcpy(ip_header.source_ip, local_ip, 4);
		memcpy(ip_header.dest_ip, bcast_ip, 4);
		ip_header.protocol = UDP_PROTOCOL;
		ip_header.length = position;

		udp_send(&udp_header);
	} // end if need_to_send

	// Clear any int1 interrupts that might have happened since
	GIFR = _BV(INTF1);
} // end SIGNAL

void broadcast(uint8_t * string, uint8_t length) {
	IPV4_HEADER ip_header;
	UDP_HEADER udp_header;
	uint16_t position = IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH;

	// Copy the message prefix into the buffer
	memcpy(data + position, string, length);
	position += length;

	// Fill the UDP header
	udp_header.ip_header = &ip_header;
	udp_header.source_port = PORT;
	udp_header.dest_port = PORT;
	// UDP header length: num of octets in udp header + data
	udp_header.length = position - IPV4_HEADER_LENGTH;

	// Fill the IP header
	memcpy(ip_header.source_ip, local_ip, 4);
	memcpy(ip_header.dest_ip, bcast_ip, 4);
	ip_header.protocol = UDP_PROTOCOL;
	// IP header length: num of octets in ip header + udp header + data
	ip_header.length = position;

	// Send it!
	udp_send(&udp_header);
}
