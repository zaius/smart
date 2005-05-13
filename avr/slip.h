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
 * \file avr/slip.h
 * \author David Kelso - david@kelso.id.au
 * \brief SLIP device driver header
 */

// SLIP special characters
/// Character used to signal the start or end of a frame
#define SLIP_END            0xC0
/// Character used to escape the use of special characters in the data
#define SLIP_ESC            0xDB
/// Character used to replace a SLIP_END character in the data
#define SLIP_ESC_END        0xDC
/// Character used to replace a SLIP_ESC character in the data
#define SLIP_ESC_ESC        0xDD

/// The maximum number of times we should retry before giving up on a
/// colliding packet
#define MAX_RETRIES 10

/// The port the RS485 transmit enable is connected to
#define TXEN_PORT PORTD
/// The pin of TXEN_PORT that the RS485 transmit enable is connected to
#define TXEN_PIN 7



// Prototypes

/**
 * slip_send - 
 * Send a packet from the data buffer over the slip device.
 * Blocks until the entire packet is finished
 */
uint8_t slip_send(void);

/**
 * slip_poll - 
 * Poll for a packet on the slip device. If there is a packet available, pass
 * it up to the corresponding network layer.
 */
void slip_poll(void);

void msleep(uint16_t);

/**
 * slip_getc - 
 * Get a character from the serial device
 * \param c a pointer to a character. If there is a character available it
 * will be set to point to that character
 * \return 1 if there was a character, 0 if nothing was available
 */
uint8_t slip_getc(uint8_t *);

/**
 * slip_putc -
 * Put a character from slip on to the serial device
 * \param c the character to send on the device
 */
uint8_t slip_putc(uint8_t);
