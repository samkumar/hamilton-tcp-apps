#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "../common.h"

#define DEFAULT_SENDTO_IP "2001:470:4889:115:212:6d07:0:113"

void ondone(int fd) {
    struct benchmark_stats stats;
    uint8_t* statsbuf = (uint8_t*) &stats;
    int rcvd = 0;
    while (rcvd < sizeof(stats)) {
        int r = recv(fd, statsbuf + rcvd, sizeof(stats) - rcvd, 0);
        if (r < 0) {
            perror("read stats");
            return;
        } else if (r == 0) {
            printf("read() of stats reached EOF\n");
            return;
        } else {
            rcvd += r;
        }
    }
    print_stats(&stats);
}

int main(int argc, char** argv) {
    const char* receiver_ip = getenv("RECEIVER_IP");
    if (receiver_ip == NULL) {
        receiver_ip = DEFAULT_SENDTO_IP;
    }
    int rv = tcp_sender(receiver_ip, ondone);
    if (rv != 0) {
        printf("Program did not properly terminate: %d\n", rv);
        return 1;
    }
    return 0;
}
