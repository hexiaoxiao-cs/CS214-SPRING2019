#include "network.h"
#include "ds.h"

#include <signal.h>
#include <stdio.h>
#include <errno.h>

void signal_handler(int a) {
    pthread_mutex_lock(&bailout_mtx);
    bailout = 1;
    pthread_mutex_unlock(&bailout_mtx);
}



int main() {
    signal(SIGINT, signal_handler);
    start_server("0.0.0.0", 3333);
    pthread_join(listener_thread_id, NULL);
}