/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * i0.h - Internet Zero application layer header file
 */

#define PORT 2134

//Prototypes
void i0_init(void);
void i0_callback(UDP_HEADER * header_in);
