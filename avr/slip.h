/**
 * Internet Zero - AVR Implementation
 * 
 * \file avr/slip.h
 * \author David Kelso - david@kelso.id.au
 * \brief SLIP device driver header
 */

// SLIP special characters
/// Character used to signal the start or end of a frame
#define SLIP_END            0xC0
/// Character used to escape the use of special characters in the data
#define SLIP_ESC            0xDB
/// Character used to replace a SLIP_END character in the data
#define SLIP_ESC_END        0xDC
/// Character used to replace a SLIP_ESC character in the data
#define SLIP_ESC_ESC        0xDD


// Prototypes

/**
 * slip_send - 
 * Send a packet from the data buffer over the slip device.
 * Blocks until the entire packet is finished
 */
void slip_send(void);

/**
 * slip_poll - 
 * Poll for a packet on the slip device. If there is a packet available, pass
 * it up to the corresponding network layer.
 */
void slip_poll(void);

/**
 * slip_getc - 
 * Get a character from the serial device
 * \param c a pointer to a character. If there is a character available it
 * will be set to point to that character
 * \return 1 if there was a character, 0 if nothing was available
 */
uint8_t slip_getc(uint8_t *c);

/**
 * slip_putc -
 * Put a character from slip on to the serial device
 * \param c the character to send on the device
 */
void    slip_putc(uint8_t  c);
