#include <sys/types.h>
#include "slip.h"

size_t slip_encode(char ** dest, char * source, size_t length) {
    int i, j = 0;
	// If someone handed a packet with characters that all needed 
	// escaping, there would be twice as many characters plus a 
	// SLIP_END character at the start and end.
	char buffer[length * 2 + 2];
	unsigned char c;

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


/*
 * Poll for a packet on the slip device. If there is a packet available, pass
 * it up to the corresponding network layer.
 */
int slip_poll(char * data, int maxlength) {
    uint8_t c, previous;
    int length = 0;

    // Keep looping while we have characters
    while (1) {
        // slip_getc(&c);

        if (c == SLIP_ESC) {
            previous = c;
            continue;
        }
        if (c == SLIP_END) {
            // End marker found, we have a complete packet
            return length;
        }

        if (previous == SLIP_ESC) {
            // Previous read byte was an escape byte, so this byte will be
            // interpreted differently from others.
            if (c == SLIP_ESC_END)
                c = SLIP_END;
            if (c == SLIP_ESC_ESC)
                c = SLIP_ESC;
        } 

        previous = c;

        // If we aren't over the length, add it to the data
        if (length < maxlength) {
            data[length] = c;
            length++;
        }
    }
}
