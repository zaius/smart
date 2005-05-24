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
 * Smart Framework - Unix Gateway
 * 
 * \file unix/main.c
 * \author David Kelso - david@kelso.id.au
 * \brief Unix gateway main loop
 */

// Use the tun network device
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>	// ioctl configuration of tunnel

#include <net/if_tun.h>	// tunnel interface
#include <net/if.h>		// ifeq struct

#include <fcntl.h>		// open
#include <stdio.h>		// printf
#include <stdlib.h>		// malloc, random
#include <unistd.h>		// write
#include <poll.h>		// poll
#include <signal.h>		// signals
#include <err.h>		// err/warn
#include <string.h>		// strlcpy

#include <time.h>		// time
#include <unistd.h>		// usleep

// Local includes
#include "main.h"
#include "slip.h"
#include "decode.h"

// stuff for inet_addr
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4000

// Amount to read on a serial read
#define S_READ 255
#define SERIAL_DEV "/dev/cuaa1"

#define TUNNEL_DEV "/dev/tun2"
#define TUNNEL_IF "tun2"
#define TUNNEL_MTU 1500
#define TUNNEL_BAUD 9600

#define IP_ADDR "10.0.0.1"
#define BROAD_ADDR "10.0.0.255"
#define NET_MASK "255.255.255.0"

#define TUNNEL_INDEX 0
#define SERIAL_INDEX 1

#define MAX_RETRIES 10

void serial_event(void);
void serial_init(void);
void tunnel_event(void);
void tunnel_init(void);

// Signal handler for CTRL-C
volatile sig_atomic_t quit = FALSE;
void sigterm(int signal) {
	quit = TRUE;
}

struct pollfd fds[2];
int serial, tunnel;
uint8_t buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
	// Seed the random number generator
	srandom(time(NULL));

	// Set up signal handlers
	signal(SIGINT, sigterm);

	serial_init();
	tunnel_init();

	// Start the main loop
	while (quit == FALSE) {
		int result = 0;

		result = poll(fds, 2, 2500);

		if (result < 0) {
			// error in poll, warn
			warn("Poll Error");
			continue;
		}

		// timed out, continue the loop
		if (result == 0) {
			printf("timeout\n");
			continue;
		}

		if (fds[SERIAL_INDEX].revents != 0) {
			serial_event();
		}

		if (fds[TUNNEL_INDEX].revents != 0) {
			tunnel_event();
		}
	}
	// We must have received a signal, tidy up and exit
	printf("Exiting...\n");
	close(serial);
	close(tunnel);

	return 0;
}

void serial_init() {
	// Open the serial interface
	serial = open(SERIAL_DEV, O_RDWR);

	if (serial < 0) {
		err(serial, "Error opening serial");
	}

	// Put the details of the serial interface into the fd array for polling
	fds[SERIAL_INDEX].fd = serial;
	fds[SERIAL_INDEX].events = POLLIN;
}

void serial_event() {
	int length, ready,i ;

	printf("serial\n");

	length = read(serial, buffer, S_READ);

	// Print the data
	printf("Encoded: ");
	for (i = 0; i < length; i++) {
		printf("0x%02x ", buffer[i]);
	}	
	printf("\n");


	// Put the data into the slip queue and read off the number of 
	// ready packets
	ready = slip_add_data(buffer, length);

	// Send all the ready packets
	while (ready > 0) {
		size_t dest_size = 4000, length;
		uint8_t dest[dest_size];

		length = slip_retrieve(dest, dest_size);

		write(tunnel, dest, length);

		ready--;

		printf("Decoded: ");
		decode(dest, length);
		for (i = 0; i < length; i++) {
			printf("0x%02x ", dest[i]);
		}
		printf("\n");
	}
}

void tunnel_init() {
	struct tuninfo tunnelinfo;
	int i, sock;
	struct ifaliasreq ifa;
	struct sockaddr_in *in;

	// Open the tunnel interface
	tunnel = open(TUNNEL_DEV, O_RDWR);

	if (tunnel < 0) 
		err(tunnel, "Error opening tunnel");

	tunnelinfo.baudrate = TUNNEL_BAUD;	// linespeed 
	tunnelinfo.mtu = TUNNEL_MTU;   		// maximum transmission unit 
	// tunnelinfo.type = ;     		// ethernet, tokenring, etc.

	// Configure the tunnel
	i = ioctl(tunnel, TUNSIFINFO, &tunnelinfo);
	if (i == -1) warn("TUNSIFINFO");
	i = ioctl(tunnel, TUNGIFINFO, &tunnelinfo);
	if (i == -1) warn("TUNGIFINFO");

	i = IFF_BROADCAST;
	i = ioctl(tunnel, TUNSIFMODE, &i);
	if (i == -1) warn("TUNSIFMODE");

#if defined __FreeBSD__
	// Prepend packets with the destination address
	i = 0;
	i = ioctl(tunnel, TUNSLMODE, &i);
	if (i == -1) warn("TUNSLMODE");
#endif

	// create a socket in order to configure the interface
	sock = socket(AF_INET, SOCK_RAW, 0);
	if (sock == -1)	warn("opening socket");	

	// Set the name of the interface we're referring to
	strlcpy(ifa.ifra_name, TUNNEL_IF, IFNAMSIZ);

	// Set the IP Address
	in = (struct sockaddr_in *)&ifa.ifra_addr;
	in->sin_family = AF_INET;
	in->sin_len = sizeof(ifa.ifra_addr);
	in->sin_addr.s_addr = inet_addr(IP_ADDR);

	// Set the Broadcast Address
	in = (struct sockaddr_in *)&ifa.ifra_broadaddr;
	in->sin_family = AF_INET;
	in->sin_len = sizeof(ifa.ifra_addr);
	in->sin_addr.s_addr = inet_addr(BROAD_ADDR);

	// Set the net mask
	in = (struct sockaddr_in *)&ifa.ifra_mask;
	in->sin_family = AF_INET;
	in->sin_len = sizeof(ifa.ifra_addr);
	in->sin_addr.s_addr = inet_addr(NET_MASK);

	// Assign the addresses to the interface
	i = ioctl(sock, SIOCAIFADDR, &ifa);
	if (i == -1) warn("SIOCAIFADDR");


	// Put the details of the tunnel interface into the fd array for polling
	fds[TUNNEL_INDEX].fd = tunnel;
	fds[TUNNEL_INDEX].events = POLLIN;
}

void tunnel_event() {
	int length, i; 
	uint8_t * encoded;
	size_t encoded_size;
	int retries = 0;

	printf("Network\n");

	// tun always promises to give a full packet which makes life easy
	length = read(tunnel, buffer, BUFFER_SIZE);

	// write(tunnel, buffer, length);

	decode(buffer, length);
	for (i = 0; i < length; i++) {
		printf("0x%02x ", buffer[i]);
	}	
	printf("\n");

	// If someone handed a packet with characters that all needed
	// escaping, there would be twice as many characters plus a
	// SLIP_END character at the start and end.
	encoded_size = length * 2 + 2;
	encoded = malloc(encoded_size * sizeof(uint8_t));

	// Encode the buffer with slip
	length = slip_encode(encoded, encoded_size, buffer, length);
	if (length < 0)
		warn("Encoded buffer too small");

	printf("Encoded: ");
	for (i = 0; i < length; i++) {
		printf("0x%02x ", encoded[i]);
	}
	printf("\n");

	// Need to IOCTL here to clear DTR (cleared means 1)
	ioctl(serial, TIOCCDTR);
	// Write the packet to the serial device
	for (i = 0; i < length; i++) {
		uint8_t received;
		// Write the character
		write(serial, encoded + i, 1);
		// Because the transmit and receive wires are tied, we 
		// should receive the same character as we sent
		read(serial, &received, 1);

		printf("0x%x\t0x%x\n", encoded[i], received);

		// If they don't match, we've had a collision
		if (encoded[i] != received) {
			printf("Collision\n");

			retries++;
			if (retries > MAX_RETRIES) break;

			// increase the wait time each time
			usleep(random() * retries);
			i = 0;
		}

		// Reset the retries in case we've had collisions
		retries = 0;
	}
	// Set the DTR (ie 0v)
	ioctl(serial, TIOCSDTR);

	free(encoded);
}
