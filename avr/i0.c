/**
 * Internet Zero - AVR Implementation
 *
 * \file avr/i0.c
 * \author David Kelso - david@kelso.id.au
 * \brief Internet Zero application
 */

#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "app.h"

// For malloc and free
#include <stdlib.h>

// Nodes for the linked list of other devices
typedef struct i0_node I0_NODE;
struct i0_node {
	uint8_t address[4];
	uint16_t port;
	uint8_t age;
	I0_NODE * next;
}


void i0_init(void) {
	udp_listen(PORT, &i0_callback);
}


void i0_callback(UDP_HEADER * header_in) {
}


void node_register() {
}


void node_forget() {
}

/*
 * node_age - periodically age the entries for the nodes
 */
/*
void node_age() { 
}
*/