/**
 * Internet Zero - AVR Implementation
 * 
 * Application layer header file
 *
 * \file app.h
 * \author David Kelso - david@kelso.id.au
 */

/// The port the application will listen on
#define PORT 1337

// Prototypes
void app_init(void);
void app_callback(UDP_HEADER * header_in);
