#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

int tcp_receiver(void (*onaccept)(void), void (*onfinished)(int)) {
    static char recvbuffer[READ_SIZE];

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

    for (;;) {
        size_t total_received = 0;
        int fd = accept(sock, NULL, 0);
        if (fd == -1) {
            perror("accept");
            break;
        } else {
            printf("Accepted connection\n");

            if (onaccept != NULL) {
                onaccept();
            }
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

            if (onfinished != NULL) {
                onfinished(fd);
            }

            rv = close(fd);
            if (rv == -1) {
                perror("close");
                return -1;
            }
        }
    }

    close(sock);
    return -1;
}

int tcp_sender(const char* receiver_ip, void (*ondone)(int)) {
    static char sendbuffer[BLOCK_SIZE];
    static char* data = "The Internet is the global system of interconnected computer networks that use the Internet protocol suite (TCP/IP) to link billions of devices worldwide. It is a network of networks that consists of millions of private, public, academic, business, and government networks of local to global scope, linked by a broad array of electronic, wireless, and optical networking technologies. The Internet carries an extensive range of information resources and services, such as the inter-linked hypertext documents and applications of the World Wide Web (WWW), electronic mail, telephony, and peer-to-peer networks for file sharing.\nAlthough the Internet protocol suite has been widely used by academia and the military industrial complex since the early 1980s, events of the late 1980s and 1990s such as more powerful and affordable computers, the advent of fiber optics, the popularization of HTTP and the Web browser, and a push towards opening the technology to commerce eventually incorporated its services and technologies into virtually every aspect of contemporary life.\nThe impact of the Internet has been so immense that it has been referred to as the '8th continent'.\nThe origins of the Internet date back to research and development commissioned by the United States government, the Government of the UK and France in the 1960s to build robust, fault-tolerant communication via computer networks. This work, led to the primary precursor networks, the ARPANET, in the United States, the Mark 1 NPL network in the United Kingdom and CYCLADES in France. The interconnection of regional academic networks in the 1980s marks the beginning of the transition to the modern Internet. From the late 1980s onward, the network experienced sustained exponential growth as generations of institutional, personal, and mobile computers were connected to it.\nInternet use grew rapidly in the West from the mid-1990s and from the late 1990s in the developing world. In the 20 years since 1995, Internet use has grown 100-times, measured for the period of one ";

    /* Fill up the sendbuffer. */
    strncpy(sendbuffer, data, BLOCK_SIZE);

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in6 receiver;
    receiver.sin6_family = AF_INET6;
    receiver.sin6_port = htons(RECEIVER_PORT);

    int rv = inet_pton(AF_INET6, receiver_ip, &receiver.sin6_addr);
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
    size_t total_sent = 0;
    for (i = 0; i != NUM_BLOCKS; i++) {
        rv = send(sock, sendbuffer, BLOCK_SIZE, 0);
        if (rv == -1) {
            perror("send");
            return -1;
        }
        total_sent += BLOCK_SIZE;
        if ((total_sent & 0x3FF) == 0) {
            printf("Sent: %d\n", (int) total_sent);
        }
    }

    if (ondone != NULL) {
        ondone(sock);
    }

    rv = close(sock);
    if (rv == -1) {
        perror("close");
        return -1;
    }

    printf("Closed socket\n");

    return 0;
}

void print_stats(struct benchmark_stats* stats) {
    printf("Microseconds: %" PRIu64 "\n", stats->time_micros);

    double sectime = ((double) stats->time_micros) / 1000000.0;
    double bytespersec = ((double) TOTAL_BYTES) / sectime;
    double kbitspersec = (8.0 * bytespersec) / 1000.0;
    printf("Bandwidth: %f kb/s\n", kbitspersec);
}
