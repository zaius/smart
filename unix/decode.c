#include <stdio.h>
#include <inttypes.h>

void decode(uint8_t * data, size_t length) {
	uint8_t position = 0, i;
	unsigned long checksum = 0;
	int temp;

	// Header Checksum...
	// From RFC1071: To check a checksum, the 1's complement sum is computed 
	// over the same set of octets, including the checksum field.  If the 
	// result is all 1 bits (-0 in 1's complement arithmetic), the check 
	// succeeds.
	i = length;
	while (i > 1) {
		uint16_t calc;
		calc  = data[--i];
		calc += data[--i] << 8;

		checksum += calc;
	}
	checksum = ~checksum;

	// Drop the packet if the checksum doesn't match
	if ((checksum & 0xffff) != 0xffff) {
		printf("Packet failed IPv4 header checksum\n");
		printf("Calculated: %lx\n", checksum);
	}

	// Version and Header Length
	printf("version: %02x\n", data[position] >> 4);
	printf("header length: %02x\n", data[position] & 0xf0);
	position++;

	// Type of Service
	printf("Type of Service: %02x\n", data[position++]);

	// Total Length
	temp = data[position++] << 8;
	temp += data[position++];
	printf("Total Length: %02x\n", temp);

	// Identification
	position += 2;

	// 3 bit flags, 5 upper fragment offset bits
	position++;
	// 8 lower fragment offset bits
	position++;

	// Time to Live
	position++;

	// Protocol
	printf("Protocol: %02x\n", data[position++]);

	// Checksum
	temp = data[position++] << 8;
	temp += data[position++];
	printf("Checksum: %02x\n", temp);

	// Source address
	printf("Source Address: %d.%d.%d.%d\n", data[position], data[position+1], data[position+2], data[position+3]);
	position += 4;
	
	// Destination address
	printf("Destination Address: %d.%d.%d.%d\n", data[position], data[position+1], data[position+2], data[position+3]);
	position += 4;
}
