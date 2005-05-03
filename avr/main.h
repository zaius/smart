/*
 * Copyright (c) 2005 David Kelso <david@kelso.id.au>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * Smart Framework - AVR Implementation
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
