/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * udp.h - UDP transport layer header file
 */

// Define the protocol for UDP in the IP header
#include <inttypes.h>

#define UDP_PROTOCOL 17
#define UDP_MAX_APPS 10
#define UDP_HEADER_LENGTH 8

typedef struct udp_header UDP_HEADER;
struct udp_header {
	IPV4_HEADER * ip_header;
	uint16_t source_port;
	uint16_t remote_port;
	uint16_t length;
};

// Prototypes

// Method used for passing packet down to ip for sending
void udp_send(UDP_HEADER * header);

// Method used for passing the packet up from the network layer
void udp_receive(IPV4_HEADER * header);

// Method used for application to register with udp 
void udp_listen(
	uint16_t port,
	void (*callback)(UDP_HEADER * header));

// Method used for application to deregister
void udp_forget(uint16_t port);
