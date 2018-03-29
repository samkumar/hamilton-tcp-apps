#include <error.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define RECEIVER_PORT 40000
#define NUM_CONNECTIONS 14
#define READ_SIZE 512

struct conninfo {
    /* Filled in by the main thread. */
    int index;
    struct sockaddr_in6 addr;
    socklen_t addr_len;
    int conn_fd;
    pthread_t thread_id;
    char addr_str[INET6_ADDRSTRLEN];

    /* Filled in by the receiving thread. */
    uint64_t total_bytes_received;
    uint64_t elapsed_nanoseconds;
};

struct conninfo connections[NUM_CONNECTIONS];

void* handle_connection(void* arg) {
    struct conninfo* info = arg;

    struct timespec start_time;
    struct timespec end_time;

    char recvbuffer[READ_SIZE];

    int rv = clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
    if (rv != 0) {
        perror("clock_gettime");
        exit(9);
    }

    ssize_t r;
    do {
        r = recv(info->conn_fd, &recvbuffer, READ_SIZE, 0);
        if (r < 0) {
            perror("read");
            exit(10);
        } else if (r != 0) {
            info->total_bytes_received += r;
        }
    } while (r != 0);

    rv = clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
    if (rv != 0) {
        perror("clock_gettime");
        exit(11);
    }

    rv = close(info->conn_fd);
    if (rv != 0) {
        perror("close");
        exit(12);
    }

    time_t seconds = end_time.tv_sec - start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    info->elapsed_nanoseconds = (UINT64_C(1000000000) * (uint64_t) seconds) + (uint64_t) nanoseconds;

    printf("Connection %d from %s ended\n", info->index, info->addr_str);

    pthread_exit(NULL);

    /* Not reached. */
    return NULL;
}

int main(int argc, char** argv) {
    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in6 local;
    memset(&local, 0x00, sizeof(local));
    local.sin6_family = AF_INET6;
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(RECEIVER_PORT);

    int rv = bind(sock, (struct sockaddr*) &local, sizeof(struct sockaddr_in6));
    if (rv == -1) {
        perror("bind");
        return 2;
    }

    rv = listen(sock, 3);
    if (rv == -1) {
        perror("listen");
        return 3;
    }

    memset(connections, 0x00, sizeof(connections));

    int i;
    for (i = 0; i != NUM_CONNECTIONS; i++) {
        struct conninfo* info = &connections[i];
        info->index = i;
        info->addr_len = sizeof(info->addr);
        info->conn_fd = accept(sock, (struct sockaddr*) &info->addr, &info->addr_len);
        if (info->conn_fd == -1) {
            perror("accept");
            return 4;
        }

        const char* dst = inet_ntop(AF_INET6, &info->addr.sin6_addr, info->addr_str, sizeof(info->addr_str));
        if (dst == NULL) {
            perror("inet_ntop");
            return 5;
        }
        printf("Accepted connection from %s\n", dst);

        rv = pthread_create(&info->thread_id, NULL, handle_connection, info);
        if (rv != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(rv));
            return 6;
        }
    }

    rv = close(sock);
    if (rv != 0) {
        perror("close");
        return 7;
    }

    for (i = 0; i != NUM_CONNECTIONS; i++) {
        struct conninfo* info = &connections[i];
        rv = pthread_join(info->thread_id, NULL);
        if (rv != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(rv));
            return 8;
        }
    }

    for (i = 0; i != NUM_CONNECTIONS; i++) {
        struct conninfo* info = &connections[i];
        double throughput = ((double) (info->total_bytes_received << 3)) / (info->elapsed_nanoseconds / 1000000.0);
        printf("[Final] %s %" PRIu64 " %" PRIu64 " (%f kb/s)\n", info->addr_str, info->total_bytes_received, info->elapsed_nanoseconds, throughput);
    }

    return 0;
}
