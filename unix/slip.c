#include "slip.h"
#include "slipchar.h"


void slip_send(char * data, int length) {
    int i, c;

    slip_putc(SLIP_END);

    for (i = 0; i < length; i++) {
        c = data[i];

        if (c == SLIP_END) {
            slip_putc(SLIP_ESC);
            slip_putc(SLIP_ESC_END);
        }
        else if (c == SLIP_ESC) {
            slip_putc(SLIP_ESC);
            slip_putc(SLIP_ESC_ESC);
        }
        else {
            slip_putc(c);
        }
    }

    slip_putc(SLIP_END);
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
        slip_getc(&c);

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
