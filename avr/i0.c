/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * i0.c - Internet Zero application
 */
#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "app.h"

// For malloc and free
#include <stdlib.h>

typedef struct i0_node I0_NODE;
struct i0_node {
	uint8_t address[4];
	uint16_t port;
	uint8_t age;
	I0_NODE * next;
}

/*
 * i0_init - initialise the application - start listening on a specified port
 * and load the existing entries out of eeprom into the linked list
 */
void i0_init(void) {
	udp_listen(PORT, &i0_callback);
}

/*
 * i0_callback - process an incoming packet
 */
void i0_callback(UDP_HEADER * header_in) {
}

/*
 * node_register - an incoming packet is from another node looking to register
 * with this node. Add the entry to eeprom
 */
void node_register() {
}

/*
 * node_forget - an incoming packet is from another node telling us to
 * deregister with this node. Delete the entry from eeprom
 */
void node_forget() {
}

/*
 * node_age - periodically age the entries for the nodes
 */
/*
void node_age() { 
}
*/