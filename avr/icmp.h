/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * icmp.h - Internet Control Message Protocol network layer header
 * RFC: http://www.ietf.org/rfc/rfc792.txt
 */

// Define the protocol for ICMP in the IP header
#define ICMP_PROTOCOL 1


#define ECHO_REPLY 0
#define ECHO_REQUEST 8

struct icmp_header {
	uint8_t type;
	uint16_t identifier;
	uint16_t seq_num;
};

// Prototypes
// uint8_t add_icmp_header(struct icmp_header * header);
