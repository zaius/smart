// Use the tun network device
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if_tun.h>
#include <sys/ioctl.h>	// ioctl configuration of tunnel

#include <net/if.h>

#include <fcntl.h>		// open
#include <stdio.h>		// printf
#include <stdlib.h>		// malloc
#include <unistd.h>		// write
#include <poll.h>		// poll
#include <signal.h>		// signals
#include <err.h>		// err/warn

// Local includes
#include "main.h"
#include "slip.h"

// Amount to read on a serial read
#define S_READ 255
#define SERIAL_DEV "/dev/cuaa0"
#define TUNNEL_DEV "/dev/tun"

// Signal handler for CTRL-C
volatile sig_atomic_t quit = FALSE;
void sigterm(int signal) {
	quit = TRUE;
}

int main(int argc, char **argv) {
	struct tuninfo tunnelinfo;
	int tunnel, serial, i;
	struct pollfd fds[2];
	char buffer[S_READ];
	struct ifreq ifr;

	// Set up signal handlers
	signal(SIGINT, sigterm);

	// Open the serial interface
	serial = open(SERIAL_DEV, O_RDWR);

	if (serial < 0) {
		err(serial, "Error opening serial");
	}
	
	// Put the details of the serial interface into the fd array for polling
	fds[1].fd = serial;
	fds[1].events = POLLIN;
    
	
	// Open the tunnel interface
	tunnel = open(TUNNEL_DEV, O_RDWR);

	if (tunnel < 0) {
		err(tunnel, "Error opening tunnel");
	}

	tunnelinfo.baudrate = 9600;               /* linespeed */
	tunnelinfo.mtu = 1500;                    /* maximum transmission unit */
	// tunnelinfo.type = ;                   /* ethernet, tokenring, etc. */

	// Configure the tunnel
	ioctl(tunnel, TUNSIFINFO, &tunnelinfo);
	ioctl(tunnel, TUNGIFINFO, &tunnelinfo);

	ioctl(tunnel, TUNSIFMODE, IFF_POINTOPOINT);
	// ifr.ifr_name = "tunl0";
	
	
	ioctl(tunnel, SIOCSIFADDR, "10.10.10.10"); // &ifr);
	ioctl(tunnel, SIOCSIFNETMASK, "255.255.255.0");
	// Put the details of the tunnel interface into the fd array for polling
	fds[0].fd = tunnel;
	fds[0].events = POLLIN;



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
		
		if (fds[serial].revents != 0) {
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
		
		if (fds[tunnel].revents != 0) {
			printf("Network\n");
		}
		
		
	}
	
	// We must have received a signal, tidy up and exit
	printf("Exiting...\n");
	close(serial);
	close(tunnel);

	return 0;
}
