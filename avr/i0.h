/**
 * Internet Zero - AVR Implementation
 *
 * \file avr/i0.h
 * \author David Kelso - david@kelso.id.au
 * \brief Internet Zero header
 */

/// The port for the i0 application to listen on
#define PORT 1337

//Prototypes

/**
 * i0_init - 
 * initialise the application - start listening on a specified port
 * and load the existing entries out of eeprom into the linked list
 */
void i0_init(void);

/**
 * i0_callback - 
 * The function to call when a packet arrives for this application
 * \param header_in A UDP header with all the data contained in the packet
 */
void i0_callback(UDP_HEADER * header_in);

/**
 * node_register - 
 * an incoming packet is from another node looking to register
 * with this node. Add the entry to eeprom
 */
void node_register();

/**
 * node_forget - 
 * an incoming packet is from another node telling us to
 * deregister with this node. Delete the entry from eeprom
 */
void node_forget();
