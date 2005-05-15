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
 * \file avr/light.c
 * \author David Kelso - david@kelso.id.au
 * \brief Functions to control a light
 */


#include "conf.h"
#include "ipv4.h"
#include "udp.h"
#include "light.h"
#include "service.h"

#include <avr/io.h>
#include <string.h> // memcmp


const uint8_t name1[4] = "turn";
const uint8_t name2[6] = "toggle";

struct service 
	//turn_service = {CONSUMER, 4, "turn", &turn_exec, 1, {BOOL}},
	// toggle_service = {CONSUMER, 6, "toggle", &toggle_exec, 0, {}};
	turn_service = {CONSUMER, 4, name1, &turn_exec, 1, {BOOL}},
	toggle_service = {CONSUMER, 6, name2, &toggle_exec, 0, {}};

struct service * services[NUM_SERVICES] = {&turn_service, &toggle_service};


void toggle_exec(uint8_t * args, uint8_t length) {
	PORTB = ~PORTB;
}

void turn_exec(uint8_t * args, uint8_t length) {
	if (!memcmp(args, "false", length))
		PORTB = 0x00;
	else if (!memcmp(args, "true", length))
		PORTB = 0xff;
	else
		log("Bad turn argument");
}
