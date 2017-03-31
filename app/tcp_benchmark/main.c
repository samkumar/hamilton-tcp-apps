#include <stdio.h>
#include <rtt_stdio.h>
#include "thread.h"
#include "xtimer.h"
#include <string.h>

#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/ipv6/autoconf_onehop.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

/* Local libraries. */
#include "init/init.h"

#include "common.h"

#define SENDTO_ADDR "2001:470:4889::114"

//#define I_AM_RECEIVER

#include "../../RIOT-OS/sys/net/gnrc/transport_layer/tcp_freebsd/bsdtcp/tcp_var.h"
extern struct tcpcb tcbs[1];
struct benchmark_stats stats;

void ondone(int fd) {
    stats.hamilton_tcp_srtt = (uint32_t) tcbs[0].t_srtt;
    stats.hamilton_tcp_rttdev = (uint32_t) tcbs[0].t_rttvar;
    stats.hamilton_tcp_rttlow = (uint32_t) tcbs[0].t_rttlow;
    stats.hamilton_tcp_cwnd = (uint32_t) tcbs[0].snd_cwnd;
    stats.hamilton_tcp_ssthresh = (uint32_t) tcbs[0].snd_ssthresh;
    stats.hamilton_tcp_total_dupacks = (uint32_t) tcbs[0].t_total_dupacks;
    /* Rest of the variables are filled in already ... */

    int rv = send(fd, &stats, sizeof(stats), 0);
    if (rv != sizeof(stats)) {
        perror("Could not send stats");
        return;
    }
}

#ifdef I_AM_RECEIVER
xtimer_ticks64_t start = {0};
void onaccept(void) {
    start = xtimer_now64();
}
void onfinished(int fd) {
    xtimer_ticks64_t end = xtimer_now64();
    /* Now, compute and print out stats. */
    xtimer_ticks64_t total_time = xtimer_diff64(end, start);
    uint64_t total_micros = xtimer_usec_from_ticks64(total_time);
    stats.time_micros = total_micros;

    ondone(fd);
}
#endif

int main(void)
{
    default_init();

    int rv;

#ifdef I_AM_RECEIVER
    printf("Running tcp_receiver program\n");
    rv = tcp_receiver(onaccept, onfinished);
#else
    printf("Waiting for 5 seconds...\n");
    xtimer_usleep(5000000L);
    printf("Running tcp_sender program\n");
    rv = tcp_sender(SENDTO_ADDR, ondone);
#endif

    if (rv == 0) {
        printf("Program is terminating!\n");
    } else {
        printf("Program did not properly terminate!\n");
    }

    while (1) {
      xtimer_usleep(10000000UL);
    }

    return 0;
}
