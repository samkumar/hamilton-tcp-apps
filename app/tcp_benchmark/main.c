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

#define RECEIVER_IP "2607:f140:400:a008:b99e:79bc:7043:f580"
#define RECEIVER_PORT 4000
#define SENDER_PORT 1024

#define BLOCK_SIZE 512
#define NUM_BLOCKS 5
char sendbuffer[BLOCK_SIZE];
char* data = "The Internet is the global system of interconnected computer networks that use the Internet protocol suite (TCP/IP) to link billions of devices worldwide. It is a network of networks that consists of millions of private, public, academic, business, and government networks of local to global scope, linked by a broad array of electronic, wireless, and optical networking technologies. The Internet carries an extensive range of information resources and services, such as the inter-linked hypertext documents and applications of the World Wide Web (WWW), electronic mail, telephony, and peer-to-peer networks for file sharing.\nAlthough the Internet protocol suite has been widely used by academia and the military industrial complex since the early 1980s, events of the late 1980s and 1990s such as more powerful and affordable computers, the advent of fiber optics, the popularization of HTTP and the Web browser, and a push towards opening the technology to commerce eventually incorporated its services and technologies into virtually every aspect of contemporary life.\nThe impact of the Internet has been so immense that it has been referred to as the '8th continent'.\nThe origins of the Internet date back to research and development commissioned by the United States government, the Government of the UK and France in the 1960s to build robust, fault-tolerant communication via computer networks. This work, led to the primary precursor networks, the ARPANET, in the United States, the Mark 1 NPL network in the United Kingdom and CYCLADES in France. The interconnection of regional academic networks in the 1980s marks the beginning of the transition to the modern Internet. From the late 1980s onward, the network experienced sustained exponential growth as generations of institutional, personal, and mobile computers were connected to it.\nInternet use grew rapidly in the West from the mid-1990s and from the late 1990s in the developing world. In the 20 years since 1995, Internet use has grown 100-times, measured for the period of one ";

#define TOTAL_BYTES (BLOCK_SIZE * NUM_BLOCKS)

#define READ_SIZE 256
char recvbuffer[READ_SIZE];

int tcp_receiver(void) {
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in6 local;
    local.sin6_family = AF_INET6;
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(RECEIVER_PORT);

    int rv = bind(sock, (struct sockaddr*) &local, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("bind");
        return -1;
    }

    rv = listen(sock, 3);
    if (rv == -1) {
        perror("listen");
        return -1;
    }

    int fd;
    size_t total_received = 0;
    while ((fd = accept(sock, NULL, 0)) != -1) {

        printf("Accepted connection\n");

        xtimer_ticks64_t start = xtimer_now64();
        while (total_received != TOTAL_BYTES) {
            size_t readsofar = 0;
            ssize_t r;
            while (readsofar != READ_SIZE) {
                r = recv(fd, &recvbuffer[readsofar], READ_SIZE - readsofar, 0);
                if (r < 0) {
                    perror("read");
                    return -1;
                } else if (r == 0) {
                    printf("read() reached EOF");
                    return -1;
                }
                readsofar += r;
            }

            total_received += READ_SIZE;

            /* Print for every 1024 bytes received. */
            if ((total_received & 0x3FF) == 0) {
                printf("Received: %d\n", (int) total_received);
            }
        }
        xtimer_ticks64_t end = xtimer_now64();

        /* Now, compute and print out stats. */
        xtimer_ticks64_t total_time = xtimer_diff64(end, start);
        uint64_t total_micros = xtimer_usec_from_ticks64(total_time);

        uint64_t seconds = total_micros / 1000000;
        uint64_t micros = total_micros % 1000000;
        double sectime = ((double) seconds) + (((double) micros) / 1000000.0);

        printf("Microseconds: %d\n", (int) total_micros);

        double bytespersec = ((double) TOTAL_BYTES) / sectime;
        double kbitspersec = (8.0 * bytespersec) / 1000.0;
        printf("Bandwidth: %f kb/s\n", kbitspersec);

        // Somehow, the conn API doesn't have a shutdown() function
        /*rv = shutdown(fd, SHUT_RDWR);
        if (rv == -1) {
            perror("shutdown");
            return -1;
        }*/

        rv = close(fd);
        if (rv == -1) {
            perror("close");
            return -1;
        }
    }

    perror("accept");
    return -1;
}

int tcp_sender(void) {
    /* Fill up the sendbuffer. */
    strncpy(sendbuffer, data, BLOCK_SIZE);

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in6 local;
    local.sin6_family = AF_INET6;
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(SENDER_PORT);

    int rv = bind(sock, (struct sockaddr*) &local, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("bind");
        return -1;
    }

    struct sockaddr_in6 receiver;
    receiver.sin6_family = AF_INET6;
    receiver.sin6_port = htons(RECEIVER_PORT);

    rv = inet_pton(AF_INET6, RECEIVER_IP, &receiver.sin6_addr);
    if (rv == -1) {
        perror("invalid address family in inet_pton");
        return -1;
    } else if (rv == 0) {
        perror("invalid ip address in inet_pton");
        return -1;
    }

    rv = connect(sock, (struct sockaddr*) &receiver, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("connect");
        return -1;
    }

    int i;
    for (i = 0; i != NUM_BLOCKS; i++) {
        rv = send(sock, sendbuffer, BLOCK_SIZE, 0);
        if (rv == -1) {
            perror("send");
            return -1;
        }
    }

    // Somehow, the conn API doesn't have a shutdown() function
    /*rv = shutdown(sock, SHUT_RDWR);
    if (rv == -1) {
        perror("shutdown");
        return -1;
    }*/

    rv = close(sock);
    if (rv == -1) {
        perror("close");
        return -1;
    }

    printf("Closed socket\n");

    return 0;
}

int main(void)
{
    default_init();

    /* Get the network addresses. */
    ipv6_addr_t addr;
    int rv = inet_pton(AF_INET6, RECEIVER_IP, &addr);
    if (rv == -1) {
        perror("invalid address family in inet_pton");
        return 1;
    } else if (rv == 0) {
        perror("invalid ip address in inet_pton");
        return 1;
    }
    kernel_pid_t intf = gnrc_ipv6_netif_find_by_addr(NULL, &addr);

    if (intf == KERNEL_PID_UNDEF) {
        printf("Running tcp_sender program\n");
        rv = tcp_sender();
    } else {
        printf("Running tcp_receiver program\n");
        rv = tcp_receiver();
    }

    if (rv == 0) {
        printf("Program is terminating!\n");
    } else {
        printf("Program did not properly terminate!\n");
    }

    while (1) {
      //Sleep
      xtimer_usleep(10000000UL);
    }

    return 0;
}
