/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * icmp.h - Internet Control Message Protocol network layer implementation
 * RFC: http://www.ietf.org/rfc/rfc792.txt
 */

uint8_t add_icmp_header(
	struct icmp_header * header, 
	uint8_t * buffer, 
	uint8_t position,
	uint8_t data_length) 
{
	// Only deal with implemented types of ICMP packets
	if (header->type != ECHO_REPLY &&
		header->type != ECHO_REQUEST)
		return 0;

	// Type
	buffer[position++] = header->type;

	// Code
	buffer[position++] = 0;

	// Zero the checksum
	buffer[position++] = 0;
	buffer[position++] = 0;

	// Identifier
	buffer[position++] = header->identifier >> 8;
	buffer[position++] = header->identifier & 0x00ff;

	// Sequence Number
	buffer[position++] = header->seq_num >> 8;
	buffer[position++] = header->seq_num & 0x00ff;

	// Calculate checksum

	return position;
}