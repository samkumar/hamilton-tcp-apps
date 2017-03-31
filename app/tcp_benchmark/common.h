#ifndef TCP_BENCHMARK_COMMON_H_
#define TCP_BENCHMARK_COMMON_H_

#include <stdbool.h>
#include <stdint.h>

#define RPI_PORT 4992
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
    uint32_t hamilton_tcp_segs_sent;
    uint32_t hamilton_tcp_segs_received; /* Not including bad checksum. */

    /* These three at the end of the experiment. */
    uint32_t hamilton_tcp_srtt;
    uint32_t hamilton_tcp_rttdev;
    uint32_t hamilton_tcp_rttlow;
    uint32_t hamilton_tcp_cwnd;
    uint32_t hamilton_tcp_ssthresh;
    uint32_t hamilton_tcp_total_dupacks;

    uint32_t hamilton_slp_packets_sent;
    uint32_t hamilton_slp_frags_received;

    uint32_t hamilton_slp_snd_packets_singlefrag;
    uint32_t hamilton_slp_frags_sent;
    uint32_t hamilton_slp_rcv_packets_singlefrag;
    uint32_t hamilton_slp_packets_reassembled;

    uint32_t hamilton_ll_frames_received;

    uint32_t hamilton_ll_retries_required[12];
    uint32_t hamilton_ll_frames_send_fail;

    uint8_t hamilton_max_retries;
} __attribute__((packed));

int tcp_receiver(void (*onaccept)(void), void (*onfinished)(int));
int tcp_sender(const char* receiver_ip, void (*ondone)(int));

int read_stats(struct benchmark_stats* stats, int fd);
int read_br_stats(struct benchmark_stats* brstats, const char* rpi_ip);
void print_stats(struct benchmark_stats* stats, bool border_router);

#endif
