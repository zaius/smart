// Use the tun network device
#include <sys/types.h>
#include <net/if_tun.h>

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

volatile sig_atomic_t quit = FALSE;
void sigterm(int signal) {
	quit = TRUE;
}

int main(int argc, char **argv) {
	int tunnel, serial, i;
	struct pollfd fds[2];
	char buffer[255];

	// Set up signal handlers
	signal(SIGINT, sigterm);

	// Open the tunnel interface
	tunnel = open("/dev/tun0", O_RDWR);

	if (tunnel < 0) {
		err(tunnel, "Error opening tunnel");
	}

	// Put the details of the tunnel interface into the fd array for polling
	fds[0].fd = tunnel;
	fds[0].events = POLLIN;

	// Open the serial interface
	serial = open("/dev/cuaa0", O_RDWR);

	if (serial < 0) {
		err(serial, "Error opening serial");
	}
	
	// Put the details of the tunnel interface into the fd array for polling
	fds[1].fd = serial;
	fds[1].events = POLLIN;
    

	// write(tunnel, "hello", 5);

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
			printf("serial\n");
			
			read(serial, buffer, 255);

			for (i = 0; i < 255; i++) {
				printf("%c", buffer[i]);
			}
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
