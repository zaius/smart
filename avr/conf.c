/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/conf.c
 * \author David Kelso - david@kelso.id.au
 * \brief Global variables
 */

#include "conf.h"

/// The buffer holding the packet
uint8_t data[MAX_DATA_SIZE];
/// The length of the data in the packet
uint16_t data_length;
