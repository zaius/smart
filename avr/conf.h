/* Internet Zero - AVR Implementation
 * David Kelso - david@kelso.id.au
 *
 * conf.h - Compile time configuration options
 */
#include <inttypes.h>


#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0


#define FREQUENCY 3686400
#define BAUD 9600

#define MAX_DATA_SIZE 100

#define SLIP TRUE
#define IPV4 TRUE
#define UDP TRUE

#define DEBUG TRUE


// The debugging log function
#define log(X)

extern uint8_t data[];
extern uint16_t data_length;
