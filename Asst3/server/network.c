#include "network.h"
#include "ds.h"

#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include <netutil.h>
#include <protocol.h>

#include <util.h>

static int server_fd;

void* network_handler_thread(void* arg);
void* listener_thread(void* arg);

int start_server(const char* hostname, unsigned int port) {
    struct sockaddr_in binder;
    int one = 1;
    if (server_fd != 0) {
        printf("Server has started already\n");
        return -1;
    }
    memset(&binder, 0, sizeof(binder));
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));
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
            pthread_create(&tmp, NULL, network_handler_thread, (void*)(size_t)new_socket);
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
    int fd = (int)(size_t)arg;
    size_t claimed_packet_size = 0;
    buffer* response_buffer;
    buffer* request_buffer = createBuffer();
    expandBuffer(request_buffer, 8 * 1024 - request_buffer->total_size);     //8K initial network buffer


    if (_poll_and_read(fd, request_buffer, sizeof(size_t)) != 0) {
        // error during poll
        pthread_exit(NULL);
    }

    // calculate proper buffer size
    memcpy((void*)&claimed_packet_size, peakBuffer(request_buffer), sizeof(size_t)); // 8 bytes for packet size
    if (claimed_packet_size > MAX_PACKET_SIZE) {
        // claimed size is too big
        TRACE(("Socket closed, reason: claimed size is too big\n"));
        goto gg_beforeprocess;
    }

    // we have a proper packet size
    // expand the buffer
    expandBuffer(request_buffer, claimed_packet_size - request_buffer->total_size);


    if (_poll_and_read(fd, request_buffer, request_buffer->total_size) != 0) {
        pthread_exit(NULL);
    }

    // transfer control to protocol layer
    if (process_packet(request_buffer, &response_buffer) == 1) {
        // protocol layer request us to close connection without responding
        // NOTE: protocol layer would free request_buffer before return
        TRACE(("Protocol layer requested to close connection\n"));
        close(fd);
        pthread_exit(NULL);
    }

    _poll_and_write(fd, response_buffer);
    close(fd);
    pthread_exit(NULL);

gg_beforeprocess:
    destroyBuffer(request_buffer);
    close(fd);
    pthread_exit(NULL);
}
