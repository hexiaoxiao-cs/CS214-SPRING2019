#include "network.h"
#include "ds.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>


static int server_fd;

void* network_handler_thread(void* arg);
void* listener_thread(void* arg);

int start_server(const char* hostname, unsigned int port) {
    struct sockaddr_in binder;
    if (server_fd != 0) {
        printf("Server has started already");
        return -1;
    }
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    binder.sin_family = AF_INET;
    binder.sin_port = htons(port);
    binder.sin_addr.s_addr = inet_addr(hostname);
    bind(server_fd, (struct sockaddr*)&binder, sizeof(binder));
    listen(server_fd, 100000);
    pthread_create(&listener_thread_id, NULL, listener_thread, NULL);
    return 0;
}

void* listener_thread(void* arg) {
    struct pollfd fds[1];
    struct sockaddr_in sin;
    pthread_t tmp;
    socklen_t sin_size;
    int poll_res;

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while((poll_res = poll(fds, 1, 1000)) >= 0) {
        if (poll_res > 0) {
            //new connection arrived
            int new_socket = accept(fds[0].fd, (struct sockaddr*)&sin, &sin_size);
            pthread_create(&tmp, NULL, network_handler_thread, (void*)new_socket);
            printf("New socket arrived\n");
        } else {
            //time out
            pthread_mutex_lock(&bailout_mtx);
            if (bailout == 1) {
                pthread_mutex_unlock(&bailout_mtx);
                printf("Signal caught, exiting\n");
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&bailout_mtx);
        }
    }
    printf("Breaked, error\n");
    pthread_exit(NULL);
}

void* network_handler_thread(void* arg) {
    int fd = (int)arg;
    int poll_res, read_res;
    char buffer[8 * 1024];  //8K of network buffer
    struct pollfd fds[1];

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while((poll_res = poll(fds, 1, 3000)) >= 0) {
        if (poll_res > 0) {
            read_res = read(fd, buffer, 8 * 1024);
            if (read_res > 0) {
                //here can send control to protocol layer
                //for dev purposes, right now it is simply a echo server
                write(fd, buffer, read_res);    //write back whatever is sent to here
            } else if (read_res == 0) {
                //client disconnected
                close(fd);
                break;
            } else {
                //error happened
                printf("Error: %s\n", strerror(errno));
                close(fd);
                break;
            }
        } else {
            //time out for poll
            close(fd);
            break;
        }
    }
    pthread_exit(NULL);
}
