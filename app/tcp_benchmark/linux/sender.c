#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common.h"

#define DEFAULT_SENDTO_IP "2001:470:4889:115:212:6d07:0:113"
#define DEFAULT_RPI_IP "2001:470:4889::115"
const char* rpi_ip;

struct benchmark_stats stats;
void ondone(int fd) {
    if (read_stats(&stats, fd) == 0) {
        print_stats(&stats, false);
    }
    struct benchmark_stats brstats;
    if (read_br_stats(&brstats, rpi_ip) == 0) {
        print_stats(&brstats, true);
    }
}

int main(int argc, char** argv) {
    const char* receiver_ip = getenv("RECEIVER_IP");
    if (receiver_ip == NULL) {
        receiver_ip = DEFAULT_SENDTO_IP;
    }
    rpi_ip = getenv("RPI_IP");
    if (rpi_ip == NULL) {
        rpi_ip = DEFAULT_RPI_IP;
    }
    int rv = tcp_sender(receiver_ip, ondone);
    if (rv != 0) {
        printf("Program did not properly terminate: %d\n", rv);
        return 1;
    }
    return 0;
}
