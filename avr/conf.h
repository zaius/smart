/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/conf.h
 * \author David Kelso - david@kelso.id.au
 * \brief Compile time configuration options
 */

#include <inttypes.h>

#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

/// The frequency that the microcontroller is running at
#define FREQUENCY 3686400
/// The baud rate to transmit at on the serial device
#define BAUD 9600

#define MAX_DATA_SIZE 100

#define SLIP TRUE
#define IPV4 TRUE
#define UDP TRUE

#define DEBUG TRUE


// The debugging log function
#define log(X)

extern uint8_t data[];
extern uint16_t data_length;
