// Use the tun network device
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if_tun.h>	// tunnel interface
#include <sys/ioctl.h>	// ioctl configuration of tunnel

#include <net/if.h>		// ifeq struct

#include <fcntl.h>		// open
#include <stdio.h>		// printf
#include <stdlib.h>		// malloc
#include <unistd.h>		// write
#include <poll.h>		// poll
#include <signal.h>		// signals
#include <err.h>		// err/warn
#include <string.h>		// strlcpy

// Local includes
#include "main.h"
#include "slip.h"

// stuff for inet_addr
#include <netinet/in.h>
#include <arpa/inet.h>
		  

// Amount to read on a serial read
#define S_READ 255
#define SERIAL_DEV "/dev/cuaa0"
#define TUNNEL_DEV "/dev/tun2"

#define TUNNEL_INDEX 0
#define SERIAL_INDEX 1

// Signal handler for CTRL-C
volatile sig_atomic_t quit = FALSE;
void sigterm(int signal) {
	quit = TRUE;
}

int main(int argc, char **argv) {
	struct tuninfo tunnelinfo;
	int tunnel, serial, i, sock;
	struct pollfd fds[2];
	char buffer[S_READ];
	//struct ifreq ifr;
	struct ifaliasreq ifa;
	struct sockaddr_in *in;

	// Set up signal handlers
	signal(SIGINT, sigterm);

	// Open the serial interface
	serial = open(SERIAL_DEV, O_RDWR);

	if (serial < 0) {
		err(serial, "Error opening serial");
	}
	
	// Put the details of the serial interface into the fd array for polling
	fds[SERIAL_INDEX].fd = serial;
	fds[SERIAL_INDEX].events = POLLIN;
	
	// Open the tunnel interface
	tunnel = open(TUNNEL_DEV, O_RDWR);

	if (tunnel < 0) {
		err(tunnel, "Error opening tunnel");
	}

	tunnelinfo.baudrate = 9600;		// linespeed 
	tunnelinfo.mtu = 1500;     		// maximum transmission unit 
	// tunnelinfo.type = ;     		// ethernet, tokenring, etc.

	// Configure the tunnel
	i = ioctl(tunnel, TUNSIFINFO, &tunnelinfo);
	if (i == -1)
		warn("TUNSIFINFO");
	i = ioctl(tunnel, TUNGIFINFO, &tunnelinfo);
	if (i == -1)
		warn("TUNGIFINFO");

	i = IFF_BROADCAST;
	i = ioctl(tunnel, TUNSIFMODE, &i);
	if (i == -1)
		warn("TUNSIFMODE");

	// Prepend packets with the destination address
	i = 1;
	i = ioctl(tunnel, TUNSLMODE, &i);
	if (i == -1)
		warn("TUNSLMODE");


	sock = socket(AF_INET, SOCK_RAW, 0);
	if (sock == -1)
		warn("opening socket");	
	// Set the IP address
	strlcpy(ifa.ifra_name, "tun1", IFNAMSIZ);
	in = (struct sockaddr_in *)&ifa.ifra_addr;
	in->sin_family = AF_INET;
	in->sin_len = sizeof(ifa.ifra_addr);
	in->sin_addr.s_addr = inet_addr("10.0.0.1");

	in = (struct sockaddr_in *)&ifa.ifra_broadaddr;
	in->sin_family = AF_INET;
	in->sin_len = sizeof(ifa.ifra_addr);
	in->sin_addr.s_addr = inet_addr("10.0.0.255");

	in = (struct sockaddr_in *)&ifa.ifra_mask;
	in->sin_family = AF_INET;
	in->sin_len = sizeof(ifa.ifra_addr);
	in->sin_addr.s_addr = inet_addr("255.255.255.0");

	i = ioctl(sock, SIOCAIFADDR, &ifa);
	if (i == -1)
		warn("SIOCAIFADDR");
/*
	
	// Set the subnet mask
	strlcpy(address.sa_data, "255.255.255.0", sizeof(address.sa_data));
	// ifr.ifr_broadaddr = inet_addr("255.255.255.0");
	address.sa_family = AF_INET;
	ifr.ifr_addr = address;
	i = ioctl(sock, SIOCSIFNETMASK, &ifr);
	if (i == -1)
		warn("SIOCSIFNETMASK");
	*/

	// Put the details of the tunnel interface into the fd array for polling
	fds[TUNNEL_INDEX].fd = tunnel;
	fds[TUNNEL_INDEX].events = POLLIN;



	// Start the main loop
	while (quit == FALSE) {
		int result = 0;
		
		result = poll(fds, 2, 500);
		
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
			int length;

			printf("serial\n");
			
			do {
				length = read(serial, buffer, S_READ);
			
				for (i = 0; i < length; i++) {
					printf("0x%02x ", buffer[i]);
				}	
			} while (length == S_READ);

			printf("\n");
		}
		
		if (fds[TUNNEL_INDEX].revents != 0) {
			printf("Network\n");
		}
	}
	
	// We must have received a signal, tidy up and exit
	printf("Exiting...\n");
	close(serial);
	close(tunnel);

	return 0;
}
