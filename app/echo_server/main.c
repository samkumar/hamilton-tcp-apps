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

#define RECEIVER_PORT 4000

#define READ_SIZE 256
char recvbuffer[READ_SIZE];

int echo_server(void) {
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
    while ((fd = accept(sock, NULL, 0)) != -1) {

        printf("Accepted connection\n");

        while (true) {
            ssize_t r = recv(fd, recvbuffer, READ_SIZE, 0);
            if (r == 0) {
                break;
            } else if (r < 0) {
                perror("recv");
                goto done;
            }

            ssize_t sent = 0;
            while (sent < r) {
                ssize_t s = send(fd, recvbuffer, r, 0);
                if (s == -1) {
                    perror("send");
                    goto done;
                }
                sent += s;
            }

        }

    done:
        rv = close(fd);
        if (rv == -1) {
            perror("close");
            return -1;
        }
    }

    perror("accept");
    rv = close(sock);
    if (rv == -1) {
        perror("close");
        return -1;
    }
    return -1;
}

int main(void)
{
    default_init();

    int rv = echo_server();

    if (rv == 0) {
        printf("Program is terminating!\n");
    } else {
        printf("Program did not properly terminate!\n");
    }

    /* Block forever */
    while (true) {
        xtimer_usleep(10000000UL);
    }

    return 0;
}
