/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/echo.h
 * \author David Kelso - david@kelso.id.au
 * \brief Simple application header file
 */

/// The port the application will listen on
#define PORT 1337

// Prototypes

/**
 * echo_init - 
 * Run time initialisation of the application
 */
void echo_init(void);

/**
 * echo_callback - 
 * The function to call when a packet arrives for this application
 * \param header_in A UDP header with all the data contained in the packet
 */
void echo_callback(UDP_HEADER * header_in);
