/**
 * Internet Zero - AVR Implementation
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
	uint8_t * source_ip;
	uint8_t dest_ip[4];
	uint8_t protocol;
	uint16_t length;
};

typedef struct ipv4_header IPV4_HEADER;

extern uint8_t local_ip[4];


// Prototypes

// ipv4_send - called by transport layer, assumes all transport layer info
// has already been inserted. Insert data from ipv4_header struct and pass it
// to the slip device to send
void ipv4_send(IPV4_HEADER *header);

// ipv4_receive - called by data link layer, strips out ipv4 info and inserts
// it into a struct. Calls upper layer
void ipv4_receive(void);
