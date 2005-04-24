/*
 * Copyright (c) 2005 David Kelso <david@kelso.id.au>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * Smart Framework - AVR Implementation
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
#define F_CPU 3686400
/// The baud rate to transmit at on the serial device
#define BAUD 9600

#define MAX_DATA_SIZE 200

#define SLIP TRUE
#define IPV4 TRUE
#define UDP TRUE

#define DEBUG TRUE


// The debugging log function
#define log(X)

extern uint8_t data[];
extern uint16_t data_length;
