/**
 * Internet Zero - AVR Implementation
 *
 * \file avr/main.h
 * \author David Kelso - david@kelso.id.au
 * \brief Main program header file
 */


// Prototypes
/** 
 * uart_init -
 * Initialise the on-chip UART 
 */
void uart_init(void);

/** 
 * timer_init - 
 * Initialise the periodic timer interrupt
 */
void timer_init(void);

/** 
 * external_init - 
 * Initialise the external button interrupt - allows us to respond to
 * external stimulus such as a button push.
 */
void external_init(void);

/** 
 * main - 
 * The main function 
 */
int main(void);

