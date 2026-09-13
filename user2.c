#include "ksocket.h"
#include <unistd.h>

int main() {
    k_socket();
    k_bind(5000);
    k_connect("127.0.0.1", 4000);

    while (1) sleep(1);
}