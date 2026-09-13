#include "ksocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

KSocket ks;

int total_messages = 0;
int total_transmissions = 0;

int dropMessage(float p) {
    float r = (float)rand() / RAND_MAX;
    return r < p;
}

/* RECEIVER THREAD */
void *receiver_thread(void *arg) {
    while (1) {
        Packet p;
        socklen_t len = sizeof(ks.peer);

        recvfrom(ks.sockfd, &p, sizeof(p), 0,
                 (struct sockaddr*)&ks.peer, &len);

        pthread_mutex_lock(&ks.lock);

        if (p.ack == 1) {
            if (p.seq >= ks.base) {
                printf("ACK received for %d\n", p.seq);
                ks.base = p.seq + 1;
            }
        } else {
            if (dropMessage(DROP_PROB)) {
                printf("Packet dropped at receiver!\n");
                pthread_mutex_unlock(&ks.lock);
                continue;
            }

            if (p.seq == ks.expected_seq) {
                printf("Received Seq %d\n", p.seq);
                ks.expected_seq++;

                Packet ack = {p.seq, 1};
                sendto(ks.sockfd, &ack, sizeof(ack), 0,
                       (struct sockaddr*)&ks.peer, sizeof(ks.peer));
            } else {
                Packet ack = {ks.expected_seq - 1, 1};
                sendto(ks.sockfd, &ack, sizeof(ack), 0,
                       (struct sockaddr*)&ks.peer, sizeof(ks.peer));
            }
        }

        pthread_mutex_unlock(&ks.lock);
    }
}

/* SENDER THREAD */
void *sender_thread(void *arg) {
    while (1) {
        sleep(1);

        pthread_mutex_lock(&ks.lock);

        for (int i = ks.base; i < ks.next_seq; i++) {
            int idx = i % WINDOW_SIZE;

            if (time(NULL) - ks.send_time[idx] >= TIMEOUT) {
                printf("Timeout! Retransmitting from %d\n", ks.base);

                for (int j = ks.base; j < ks.next_seq; j++) {
                    int id = j % WINDOW_SIZE;

                    sendto(ks.sockfd, &ks.window[id], sizeof(Packet), 0,
                           (struct sockaddr*)&ks.peer, sizeof(ks.peer));

                    ks.send_time[id] = time(NULL);
                    total_transmissions++;
                }
                break;
            }
        }

        pthread_mutex_unlock(&ks.lock);
    }
}

int k_socket() {
    ks.sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    ks.base = 0;
    ks.next_seq = 0;
    ks.expected_seq = 0;

    pthread_mutex_init(&ks.lock, NULL);

    pthread_t r, s;
    pthread_create(&r, NULL, receiver_thread, NULL);
    pthread_create(&s, NULL, sender_thread, NULL);

    return 1;
}

int k_bind(int port) {
    ks.addr.sin_family = AF_INET;
    ks.addr.sin_port = htons(port);
    ks.addr.sin_addr.s_addr = INADDR_ANY;

    bind(ks.sockfd, (struct sockaddr*)&ks.addr, sizeof(ks.addr));
    return 0;
}

int k_connect(char *ip, int port) {
    ks.peer.sin_family = AF_INET;
    ks.peer.sin_port = htons(port);
    ks.peer.sin_addr.s_addr = inet_addr(ip);
    return 0;
}

int k_sendto(char *buf, int len) {
    while (1) {
        pthread_mutex_lock(&ks.lock);

        if (ks.next_seq < ks.base + WINDOW_SIZE) {
            int idx = ks.next_seq % WINDOW_SIZE;

            Packet *p = &ks.window[idx];
            p->seq = ks.next_seq;
            p->ack = 0;
            memcpy(p->data, buf, len);

            sendto(ks.sockfd, p, sizeof(Packet), 0,
                   (struct sockaddr*)&ks.peer, sizeof(ks.peer));

            ks.send_time[idx] = time(NULL);

            printf("Sent Seq %d\n", p->seq);

            total_messages++;
            total_transmissions++;

            ks.next_seq++;

            pthread_mutex_unlock(&ks.lock);
            return len;
        }

        pthread_mutex_unlock(&ks.lock);
        usleep(10000);  // wait if window full
    }
}