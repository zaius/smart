/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/button.h
 * \author David Kelso - david@kelso.id.au
 * \brief Simple application header file
 */

/// The port the application will listen on
#define PORT 1337

// Prototypes

/**
 * button_init - 
 * Run time initialisation of the application
 */
void button_init(void);

/**
 * button_callback - 
 * The function to call when a packet arrives for this application
 * \param header_in A UDP header with all the data contained in the packet
 */
void button_callback(UDP_HEADER * header_in);
