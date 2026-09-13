#include "ksocket.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    k_socket();
    k_bind(4000);
    k_connect("127.0.0.1", 5000);

    FILE *fp = fopen("input.txt", "r");
    if (!fp) {
        perror("file error");
        return 1;
    }

    char buf[MSG_SIZE];

    while (fread(buf, 1, MSG_SIZE, fp) > 0) {
        k_sendto(buf, MSG_SIZE);
    }

    fclose(fp);

    sleep(40);   // IMPORTANT (wait for all ACKs)

    printf("\n===== RESULTS =====\n");
    printf("Total Messages: %d\n", total_messages);
    printf("Total Transmissions: %d\n", total_transmissions);

    float avg = (float)total_transmissions / total_messages;
    printf("Average Transmissions per Message: %.2f\n", avg);

    return 0;
}