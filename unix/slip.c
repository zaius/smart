#include <sys/types.h>
#include <stdlib.h>
#include "slip.h"

// Encode an array of data using SLIP encoding
size_t slip_encode(uint8_t ** dest, uint8_t * source, size_t length) {
	int i, j = 0;
	uint8_t c;
	uint8_t * buffer;

	// If someone handed a packet with characters that all needed 
	// escaping, there would be twice as many characters plus a 
	// SLIP_END character at the start and end.
	buffer = malloc((length * 2 + 2) * sizeof(uint8_t));

	// Point the destination pointer to the address of the array
	*dest = buffer;

	buffer[j++] = SLIP_END;

	for (i = 0; i < length; i++) {
		c = source[i];

		if (c == SLIP_END) {
			buffer[j++] = SLIP_ESC;
			buffer[j++] = SLIP_ESC_END;
		}
		else if (c == SLIP_ESC) {
			buffer[j++] = SLIP_ESC;
			buffer[j++] = SLIP_ESC_ESC;
		}
		else {
			buffer[j++] = c;
		}
	}

	buffer[j++] = SLIP_END;

	return j;
}

// Since a read on the serial device can never promise a full packet,
// this function copies all the data that may be read over a period of
// time and inserts it into a contiguous buffer in order to look for
// an entire packet. it returns the number of full packets that are
// available as a result of calling the slip_add_data function and the
// youngest decoded packets can be obtained by calling slip_retrieve
int slip_add_data(uint8_t * source, size_t length) {
}
size_t slip_retrieve(uint8_t ** dest) {
}

int slip_decode(uint8_t ** dest, uint8_t * source, size_t length) {
	uint8_t c, previous;
	uint8_t * buffer;
	int source_pos, dest_pos; 

	// Since we are removing characters, the destination can never be bigger
	// than the source
	buffer = malloc(length * sizeof(uint8_t));

	// Point the destination pointer to the address of the array
	*dest = buffer;


	for (source_pos = 0; source_pos < length; source_pos++) {
		c = source[source_pos];

		if (c == SLIP_ESC) {
			previous = c;
			continue;
		}

		if (previous == SLIP_ESC) {
			// Previous read byte was an escape byte, so this byte will be
			// interpreted differently from others.
			if (c == SLIP_ESC_END)
				c = SLIP_END;
			if (c == SLIP_ESC_ESC)
				c = SLIP_ESC;
		} 
		else if (c == SLIP_END) {
			// The current character is a slip end and the previous
			// character wasn't an escape - therefore we have found the
			// end marker and have a complete packet
			return dest_pos;
		}

		previous = c;

		// Add it to the data
		data[dest_pos++] = c;
	}
	
	return dest_pos;
}
