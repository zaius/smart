// Use the tun network device
#include <sys/types.h>
#include <net/if_tun.h>

#include <fcntl.h>		// open
#include <stdio.h>		// printf
#include <stdlib.h>		// malloc
#include <unistd.h>		// write
#include <poll.h>		// poll
#include <signal.h>		// signals

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
	tunnel = open("/dev/tun0", 0, 0);

	if (!tunnel) {
		printf("Error opening tunnel");
		return 1;
	}

	// Put the details of the tunnel interface into the fd array for polling
	fds[0].fd = tunnel;
	fds[0].events = POLLIN;
	fds[0].revents = POLLIN;

	// Open the serial interface
	serial = open("/dev/cuaa0", 0, 0);

	if (!serial) {
		printf("Error opening serial");
		return 1;
	}
	
	// Put the details of the tunnel interface into the fd array for polling
	fds[1].fd = serial;
	fds[1].events = POLLIN;
	fds[1].revents = POLLIN;
    

	// write(tunnel, "hello", 5);

	// Start the main loop
	while (quit == FALSE) {
		int result = 0;
		result = poll(fds, 2, 500);
		
		// timed out, continue the loop
		if (result == 0) {
			printf("hi\n");
			continue;
		}
		
		if (result == serial) {
			printf("serial\n");
			read(serial, buffer, 255);

			for (i = 0; i < 255; i++) {
				printf("%c", buffer[i]);
			}
			printf("\n");
		}
		else if (result == tunnel) {
			printf("Network\n");
		}
		else {
			// error in poll, exit
			return -1;
		}
		
	}
	
	// We must have received a signal, tidy up and exit
	printf("Exiting...\n");

	return 0;
}
