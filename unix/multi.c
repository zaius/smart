// For opening files
#include <fcntl.h>

// Use the tun network device
#include <sys/types.h>
#include <net/if_tun.h>

// printf
#include <stdio.h>

// write
#include <unistd.h>

#include <poll.h>

// Local includes
#include "slip.h"

int main(int argc, char **argv) {
    int tunnel, serial;
    struct pollfd * fds[2];

    tunnel = open("/dev/tun0", 0, 0);

    if (!tunnel) {
        printf("Error opening tunnel");
        return 1;
    }

    while (1) {
    
    }

    serial = open("/dev/cua00", 0, 0);

    if (!serial) {
        printf("Error opening serial");
        return 1;
    }
    
    write(tunnel, "hello", 5);

    return 0;
}
