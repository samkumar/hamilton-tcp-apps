#ifndef TCP_BENCHMARK_COMMON_H_
#define TCP_BENCHMARK_COMMON_H_

#include <stdint.h>

#define RECEIVER_PORT 40000
#define SENDER_PORT 40240

/* Controls sender. */
#define BLOCK_SIZE 2048
#define NUM_BLOCKS 50

#define TOTAL_BYTES (BLOCK_SIZE * NUM_BLOCKS)

/* Size of blocks in which the receiver consumes the data. */
#define READ_SIZE 512

struct benchmark_stats {
    uint64_t time_micros;
};

int tcp_receiver(void (*onaccept)(void), void (*onfinished)(int));
int tcp_sender(const char* receiver_ip, void (*ondone)(int));

void print_stats(struct benchmark_stats* stats);

#endif
