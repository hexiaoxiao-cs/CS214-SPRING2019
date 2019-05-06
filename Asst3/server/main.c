#include "network.h"
#include "ds.h"

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

void signal_handler(int a) {
    pthread_mutex_lock(&bailout_mtx);
    bailout = 1;
    pthread_mutex_unlock(&bailout_mtx);
}

int main(int argc, char* argv[]) {
    char* end;
    unsigned int port;
    if (argc != 2) {
        printf("Usage: ./WTFserver port\n");
        return 0;
    }
    port = strtol(argv[1], &end, 10);
    if (argv[1][0] != 0 && *end == 0) {
        init_hashmap();
        signal(SIGINT, signal_handler);
        if (start_server("0.0.0.0", port) == 0)
            pthread_join(listener_thread_id, NULL);
        return 0;
    } else {
        printf("Inputted port is not a number\n");
        return 0;
    }
}