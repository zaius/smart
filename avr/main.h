/**
 * Internet Zero - AVR Implementation
 *
 * \file main.h
 * \author David Kelso - david@kelso.id.au
 * \brief Main program header file
 *
 * This is some more text talking about what this file actually does.
 */

/**
 * \mainpage Internet Zero - AVR Implementation
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Usage
 *
 * \subsection step1 Step 1: Compiling
 * Tools needed for compiling are
 *	- GCC with AVR extensions
 *	- AVRLibC
 *	- Make
 *
 * For the windows platform, these are all available through the winavr
 * package available at http://winavr.sourceforge.net
 */

// Prototypes
/** Initialise the on-chip UART */
void uart_init(void);

/** Initialise the periodic timer interrupt */
void timer_init(void);

/** 
 * Initialise the external button interrupt - allows us to respond to
 * external stimulus such as a button push.
 */
void external_init(void);

/** The main function */
int main(void);

uint8_t str_cpy(char * source, uint8_t length, char * destination, uint8_t position);

