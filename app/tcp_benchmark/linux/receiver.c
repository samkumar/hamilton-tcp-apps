#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "../common.h"

#define DEFAULT_RPI_IP "2001:470:4889::115"
const char* rpi_ip;

struct timespec start;
void onaccept(void) {
    int rv = clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    if (rv != 0) {
        perror("clock_gettime");
        exit(1);
    }
}

struct benchmark_stats stats;
void onfinished(int fd) {
    struct timespec end;
    int rv = clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    if (rv != 0) {
        perror("clock_gettime");
        exit(1);
    }
    int64_t secdiff = (int64_t) (end.tv_sec - start.tv_sec);
    int64_t nanodiff = (int64_t) (end.tv_nsec - start.tv_nsec);
    uint64_t elapsed_nanos = (uint64_t) (secdiff * ((int64_t) 1000000000L) + nanodiff);
    uint64_t elapsed_micros = elapsed_nanos / 1000;
    if ((elapsed_nanos % 1000) >= 500) {
        elapsed_micros += 1;
    }

    if (read_stats(&stats, fd) != 0) {
        perror("read_stats");
    }

    stats.time_micros = elapsed_micros;

    print_stats(&stats, false);

    struct benchmark_stats brstats;
    if (read_br_stats(&brstats, rpi_ip) == 0) {
        print_stats(&brstats, true);
    }
}

int main(int argc, char** argv) {
    rpi_ip = getenv("RPI_IP");
    if (rpi_ip == NULL) {
        rpi_ip = DEFAULT_RPI_IP;
    }
    int rv = tcp_receiver(onaccept, onfinished);
    if (rv != 0) {
        printf("Program did not properly terminate: %d\n", rv);
        return 1;
    }

    return 0;
}
