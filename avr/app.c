/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/app.c
 * \author David Kelso - david@kelso.id.au
 * \brief Simple UDP echo application
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
	UDP_HEADER header_out;
	IPV4_HEADER ip_header_out;

	PORTB = ~PORTB;

	header_out.ip_header = &ip_header_out;

	// Reset the length
	data_length = header_in->ip_header->length;
	
	header_out.source_port = PORT;
	header_out.remote_port = header_in->source_port;
	header_out.length = header_in->length;

	header_out.ip_header->protocol = UDP_PROTOCOL;
	header_out.ip_header->length = data_length;

	// header_out.ip_header->source_ip = local_ip;
	header_out.ip_header->source_ip[0] = local_ip[0];
	header_out.ip_header->source_ip[1] = local_ip[1];
	header_out.ip_header->source_ip[2] = local_ip[2];
	header_out.ip_header->source_ip[3] = local_ip[3];

	header_out.ip_header->dest_ip[0] = header_in->ip_header->source_ip[0];
	header_out.ip_header->dest_ip[1] = header_in->ip_header->source_ip[1];
	header_out.ip_header->dest_ip[2] = header_in->ip_header->source_ip[2];
	header_out.ip_header->dest_ip[3] = header_in->ip_header->source_ip[3];

	udp_send(&header_out);
}
