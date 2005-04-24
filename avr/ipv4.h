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
 * \file avr/ipv4.h
 * \author David Kelso - david@kelso.id.au
 * \brief Internet Protocol (version 4) header
 *
 * Implementation details:
 * http://en.wikipedia.org/wiki/IPV4
 * http://www.ietf.org/rfc/rfc791.txt
 * http://www.ietf.org/rfc/rfc1071.txt
 * 
 */
#define IPV4_HEADER_LENGTH 20

#define VER 4
#define IHL 5
#define TOS 0
#define ID 5
#define FLAGS 0
#define FRAGMENT_OFFSET 0x0000
#define TTL 255

struct ipv4_header {
	uint8_t source_ip[4];
	uint8_t dest_ip[4];
	uint8_t protocol;
	uint16_t length;
};

typedef struct ipv4_header IPV4_HEADER;

extern uint8_t local_ip[4];
extern uint8_t bcast_ip[4];


// Prototypes
/**
* compare_ip
* Compare two uint8_t arrays containing IP addresses
* \param one A uint8_t array of length 4 to be compared to two
* \param one A uint8_t array of length 4 to be compared to one
* \returns TRUE if the arrays match, FALSE if they don't
*/
uint8_t compare_ip(uint8_t *, uint8_t *);

/**
 * ipv4_send
 * Called by the transport layer. Inserts data from ipv4_header struct 
 * in to the data buffer and then passes it to the slip device to send.
 * ipv4_send assumes all the relevant transport layer information has 
 * already been inserted into the data packet.
 * \param header A pointer to a header containing the IP information to
 * be inserted in the packet
 */
void ipv4_send(IPV4_HEADER *);

/**
 * ipv4_receive
 * Called by data link layer. Strips out the information from the data
 * array and inserts it into a struct. It then calls the relevant next
 * step in the chain (if it exists)
 */
void ipv4_receive(void);
