/**
 * Internet Zero - AVR Implementation
 *
 * \file avr/icmp.h
 * \author David Kelso - david@kelso.id.au
 * \brief Internet Control Message Protocol network layer header
 *
 * RFC: http://www.ietf.org/rfc/rfc792.txt
 */

/// The protocol number for ICMP in the IP header
#define ICMP_PROTOCOL 1


#define ECHO_REPLY 0
#define ECHO_REQUEST 8

struct icmp_header {
	uint8_t type;
	uint16_t identifier;
	uint16_t seq_num;
};

// Prototypes
