/**
 * Internet Zero - AVR Implementation
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

#include <string.h> // strncmp

/*
 * light_init - initialise the application
 */
void light_init(void) {
	/*
	if (udp_listen(PORT, &light_callback)) {
		log("port in use");
	}
	*/
	udp_listen(PORT, &light_callback);
}

/*
 * light_callback - process an incoming packet
 */
void light_callback(UDP_HEADER * header_in) {
	uint8_t * message = &data[IPV4_HEADER_LENGTH + UDP_HEADER_LENGTH];
	uint16_t message_size = header_in->length - 8;

	// TODO: Check that there are the correct number of characters 
	// before checking the message
	if (!memcmp(message, "toggle", 6)) {
		PORTB = ~PORTB;
	}
	else if (!memcmp(message, "turn 0", 6)) {
		PORTB = 0x00;
	}
	else if (!memcmp(message, "turn 1", 6)) {
		PORTB = 0xff;
	}
	else {
		log("Incorrect message received");
	}
}
