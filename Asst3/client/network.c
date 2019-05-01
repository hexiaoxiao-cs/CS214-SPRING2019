#include "network.h"
#include "protocol.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

int send_request(const char* hostname, uint16_t port, buffer* in, buffer** out) {
    struct sockaddr_in connector;
    struct pollfd fds[1];
    buffer* response_buffer;

    int poll_res, fd = socket(AF_INET, SOCK_STREAM, 0);
    ssize_t write_res, read_res;
    size_t claimed_packet_size, write_accum = 0;

    memset(&connector, 0, sizeof(connector));
    connector.sin_family = AF_INET;
    connector.sin_port = htons(port);
    connector.sin_addr.s_addr = inet_addr(hostname);

    if (connect(fd, (struct sockaddr*)&connector, sizeof(connector)) == -1) {
        printf("Connect error\n");
        return 1;
    }

    fds[0].fd = fd;
    fds[0].events = POLLOUT;

    while (1) {
        poll_res = poll(fds, 1, 3000);
        if (poll_res > 0) {
            write_res = write(fd, peakBuffer(in) + write_accum, getLengthBuffer(in) - write_accum);
            if (write_res > 0) {
                write_accum += write_res;
                if (write_accum == getLengthBuffer(in)) {
                    // sent the whole packet
                    break;  // release resources
                }
            } else if (write_res == 0) {
                //server disconnected
                goto gg_before_receive;
            } else {
                TRACE(("Read error: %d\n", strerror(errno)));
                goto gg_before_receive;
            }
        } else if (poll_res == 0) {
            // poll time out
            TRACE(("Wait for write timed out\n"));
            goto gg_before_receive;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg_before_receive;
        }
    }

    destroyBuffer(in);

    // wait for server response
    response_buffer = createBuffer();
    expandBuffer(response_buffer, 8 * 1024 - availableBuffer(response_buffer));
    fds[0].events = POLLIN;

    // poll for size first
    // POLL_IN only
    while (1) {
        poll_res = poll(fds, 1, 3000);
        if (poll_res > 0) {
            read_res = read(fd, lastposBuffer(response_buffer), availableBuffer(response_buffer));
            if (read_res > 0) {
                response_buffer->size += read_res;  // increase internal buffer size
                if (getLengthBuffer(response_buffer) >= sizeof(size_t)) {
                    // make sure we have the size ready to read
                    break;
                }
            } else if (read_res == 0) {
                //client disconnected
                goto gg_after_receive; // release resources
            } else {
                //error happened
                TRACE(("Read error: %s\n", strerror(errno)));
                goto gg_after_receive;
            }
        } else if (poll_res == 0){
            // time out for poll
            TRACE(("Socket timed out\n"));
            goto gg_after_receive;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg_after_receive;
        }
    }

    // calculate proper buffer size
    memcpy((void*)&claimed_packet_size, peakBuffer(response_buffer), 8); // 8 bytes for packet size
    if (claimed_packet_size > MAX_PACKET_SIZE) {
        // claimed size is too big
        TRACE(("Socket closed, reason: claimed size is too big\n"));
        goto gg_after_receive;
    }

    // we have a proper packet size
    // expand the buffer
    expandBuffer(response_buffer, claimed_packet_size - availableBuffer(response_buffer));

    // poll for data (POLL_IN only)
    // note: we use multiple pool to avoid condition boolean check for indication of receiving status

    while (1) {
        poll_res = poll(fds, 1, 3000);
        if (poll_res > 0) {
            read_res = read(fd, lastposBuffer(response_buffer), availableBuffer(response_buffer));
            if (read_res > 0) {
                response_buffer->size += read_res;
                // check full length of packet
                if (availableBuffer(response_buffer) == 0) {
                    //we have read the whole packet
                    break;  //transfer control to protocol layer
                }
            } else if (read_res == 0) {
                //client disconnected
                goto gg_after_receive;
            } else {
                // error
                TRACE(("Read error: %d\n", strerror(errno)));
                goto gg_after_receive;
            }
        } else if (poll_res == 0) {
            // poll time out
            TRACE(("Socket timed out\n"));
            goto gg_after_receive;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg_after_receive;
        }
    }

    *out = response_buffer;
    return 0;

gg_before_receive:
    destroyBuffer(in);
    return 1;

gg_after_receive:
    destroyBuffer(response_buffer);
    close(fd);
    return 1;
}