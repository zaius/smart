/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/app.c
 * \author David Kelso - david@kelso.id.au
 * \brief Simple application
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "app.h"
#include <avr/io.h>

// For malloc and free
#include <stdlib.h>

/*
 * app_init - initialise the application
 */
void app_init(void) {
	/*
	if (udp_listen(PORT, &app_callback)) {
		log("port in use");
	}
	*/
	udp_listen(PORT, &app_callback);
}

/*
 * app_callback - process an incoming packet
 */
void app_callback(UDP_HEADER * header_in) {
	PORTB++;
	/*
	// Reset the length
	data_length = 0;

	UDP_HEADER * header_out = malloc(sizeof(UDP_HEADER));
	header_out->ip_header = malloc(sizeof(IPV4_HEADER));
	
	data_length += IPV4_HEADER_LENGTH;
	data_length += UDP_HEADER_LENGTH;

	data[data_length++] = 't';
	data[data_length++] = 'e';
	data[data_length++] = 's';
	data[data_length++] = 't';

	header_out->source_port = PORT;
	header_out->remote_port = header_in->source_port;
	header_out->length = data_length;

	header_out->ip_header->protocol = UDP_PROTOCOL;
	header_out->ip_header->length = data_length;
	// ah.. this should use some defined ip as opposed to just swapping them
	// Also, some way to just assign the addresses of the arrays instead of
	// doing a deep copy of their values
	header_out->ip_header->local_ip[0] = header_in->ip_header->remote_ip[0];
	header_out->ip_header->local_ip[1] = header_in->ip_header->remote_ip[1];
	header_out->ip_header->local_ip[2] = header_in->ip_header->remote_ip[2];
	header_out->ip_header->local_ip[3] = header_in->ip_header->remote_ip[3];
	header_out->ip_header->remote_ip[0] = header_in->ip_header->local_ip[0];
	header_out->ip_header->remote_ip[1] = header_in->ip_header->local_ip[1];
	header_out->ip_header->remote_ip[2] = header_in->ip_header->local_ip[2];
	header_out->ip_header->remote_ip[3] = header_in->ip_header->local_ip[3];

	udp_send(header_out);

	free(header_out->ip_header);
	free(header_out);
	*/
}
