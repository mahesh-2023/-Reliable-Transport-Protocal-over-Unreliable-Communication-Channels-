#ifndef KSOCKET_H
#define KSOCKET_H

#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define MSG_SIZE 512
#define WINDOW_SIZE 5
#define TIMEOUT 2
#define DROP_PROB 0.3

typedef struct {
    int seq;
    int ack;
    char data[MSG_SIZE];
} Packet;

typedef struct {
    int sockfd;
    struct sockaddr_in addr, peer;

    int base;
    int next_seq;
    int expected_seq;

    Packet window[WINDOW_SIZE];
    time_t send_time[WINDOW_SIZE];

    pthread_mutex_t lock;

} KSocket;

extern int total_messages;
extern int total_transmissions;

int k_socket();
int k_bind(int port);
int k_connect(char *ip, int port);
int k_sendto(char *buf, int len);

#endif