#include "network.h"
#include "ds.h"
#include "util.h"
#include "protocol.h"

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
        printf("Server has started already\n");
        return -1;
    }
    memset(&binder, 0, sizeof(binder));
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
    int poll_res;
    ssize_t read_res, write_res, write_accum = 0;
    struct pollfd fds[1];
    size_t claimed_packet_size = 0;
    buffer* response_buffer;
    buffer* request_buffer = createBuffer();
    expandBuffer(request_buffer, 8 * 1024 - availableBuffer(request_buffer));     //8K initial network buffer

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    // poll for size first
    // POLL_IN only
    while (1) {
        poll_res = poll(fds, 1, 3000);
        if (poll_res > 0) {
            read_res = read(fd, lastposBuffer(request_buffer), availableBuffer(request_buffer));
            if (read_res > 0) {
                request_buffer->size += read_res;  // increase internal buffer size
                if (getLengthBuffer(request_buffer) >= sizeof(size_t)) {
                    // make sure we have the size ready to read
                    break;
                }
            } else if (read_res == 0) {
                //client disconnected
                destroyBuffer(request_buffer);
                goto gg_beforeprocess;
            } else {
                //error happened
                TRACE(("Read error: %s\n", strerror(errno)));
                goto gg_beforeprocess;
            }
        } else if (poll_res == 0){
            // time out for poll
            TRACE(("Socket timed out\n"));
            goto gg_beforeprocess;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg_beforeprocess;
        }
    }

    // calculate proper buffer size
    memcpy((void*)&claimed_packet_size, peakBuffer(request_buffer), 8); // 8 bytes for packet size
    if (claimed_packet_size > MAX_PACKET_SIZE) {
        // claimed size is too big
        TRACE(("Socket closed, reason: claimed size is too big\n"));
        goto gg_beforeprocess;
    }

    // we have a proper packet size
    // expand the buffer
    expandBuffer(request_buffer, claimed_packet_size - availableBuffer(request_buffer));

    // poll for data (POLL_IN only)
    // note: we use multiple pool to avoid condition boolean check for indication of receiving status

    while (1) {
        poll_res = poll(fds, 1, 3000);
        if (poll_res > 0) {
            read_res = read(fd, lastposBuffer(request_buffer), availableBuffer(request_buffer));
            if (read_res > 0) {
                request_buffer->size += read_res;
                // check full length of packet
                if (availableBuffer(request_buffer) == 0) {
                    //we have read the whole packet
                    break;  //transfer control to protocol layer
                }
            } else if (read_res == 0) {
                //client disconnected
                goto gg_beforeprocess;
            } else {
                // error
                TRACE(("Read error: %d\n", strerror(errno)));
                goto gg_beforeprocess;
            }
        } else if (poll_res == 0) {
            // poll time out
            TRACE(("Socket timed out\n"));
            goto gg_beforeprocess;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg_beforeprocess;
        }
    }

    // transfer control to protocol layer
    if (process_packet(request_buffer, &response_buffer) == 1) {
        // protocol layer request us to close connection without responding
        TRACE(("Protocol layer requested to close connection\n"));
        close(fd);
        pthread_exit(NULL);
    }

    // there is a response packet that need to be sent
    // POLL_OUT
    fds[0].events = POLLOUT;

    while (1) {
        poll_res = poll(fds, 1, 3000);
        if (poll_res > 0) {
            write_res = write(fd, peakBuffer(response_buffer) + write_accum, getLengthBuffer(response_buffer) - write_accum);
            if (write_res > 0) {
                write_accum += write_res;
                if (write_accum == getLengthBuffer(response_buffer)) {
                    // sent the whole packet
                    break;  // release resources
                }
            } else if (write_res == 0) {
                //client disconnected
                goto gg_afterprocess;
            } else {
                TRACE(("Read error: %d\n", strerror(errno)));
                goto gg_afterprocess;
            }
        } else if (poll_res == 0) {
            // poll time out
            TRACE(("Wait for write timed out\n"));
            goto gg_afterprocess;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg_afterprocess;
        }
    }

gg_afterprocess:
    destroyBuffer(response_buffer);
    pthread_exit(NULL);

gg_beforeprocess:
    destroyBuffer(request_buffer);
    close(fd);
    pthread_exit(NULL);
}
