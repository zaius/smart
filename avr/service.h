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
 * \file avr/service.h
 * \author David Kelso - david@kelso.id.au
 * \brief Header of application to coordinate services of smart devices
 */

/// The port the application will listen on
#define PORT 1337

#define BOOL 1
#define STRING 2
#define INT8 3
#define UINT8 4

#define CONSUMER 1
#define PRODUCER 2

struct service {
	uint8_t type;
	uint8_t name_length;
	uint8_t * name;
	void (*on_exec)(char * args, uint8_t length);
	uint8_t arg_length;
	uint8_t arguments[];
};

struct destination {
	uint8_t address[4]; // IP address of the destination
	uint16_t port; // Port of the destination
	struct service * source_service; // The local producer service
	struct service dest_service; // The remote consumer service
	struct destination * next; // The next destination in the message list
};

extern struct destination * message_list;


// Prototypes

/**
 * service_init - 
 * Run time initialisation of the application
 */
void service_init(void);

/**
 * service_callback - 
 * The function to call when a packet arrives for this application
 * \param header_in A UDP header with all the data contained in the packet
 */
void service_callback(UDP_HEADER * header_in);

/**
 * erase -
 * free all the memory in the linked list from the pointer onwards
 * \param pointer The destination pointer from which to free
 */
void erase(struct destination *);

/**
 * load -
 * load the destinations stored into eeprom in to the message list
 * starting at the pointer
 * \param pointer The destination pointer to start filling from the eeprom
 */
void load(struct destination *);

/**
 * save -
 * save the destinations in the list specified by pointer to eeprom
 * \param pointer The destination pointer with the list to save
 */
void save(struct destination *);

/**
 * debounce -
 * Eliminate the bouncing and noise when a switch is pressed and find
 * the real value.
 * \param port A pointer to the input port to take the data from
 * \param pin The number of the pin to take the value from
 * \returns The true value of the port
 */
uint8_t debounce(volatile uint8_t *port, uint8_t pin);