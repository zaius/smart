/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * slip.h - Header file for slip device driver
 */

// SLIP special characters
#define SLIP_END			0300
#define SLIP_ESC			0333
#define SLIP_ESC_END		0334
#define SLIP_ESC_ESC		0335


// Prototypes
void slip_send(void);
void slip_poll(void);

uint8_t slip_getc(uint8_t *c);
void    slip_putc(uint8_t  c);
