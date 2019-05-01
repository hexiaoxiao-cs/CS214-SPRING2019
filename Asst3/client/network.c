#include "network.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <netutil.h>
#include <protocol.h>

int send_request(const char* hostname, uint16_t port, buffer* in, buffer** out) {
    struct sockaddr_in connector;
    buffer* response_buffer;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    size_t claimed_packet_size;

    memset(&connector, 0, sizeof(connector));
    connector.sin_family = AF_INET;
    connector.sin_port = htons(port);
    connector.sin_addr.s_addr = inet_addr(hostname);

    if (connect(fd, (struct sockaddr*)&connector, sizeof(connector)) == -1) {
        printf("Connect error\n");
        return 1;
    }

    if (_poll_and_write(fd, in) != 0) {
        return 1;
    }

    // wait for server response
    response_buffer = createBuffer();
    expandBuffer(response_buffer, 8 * 1024 - availableBuffer(response_buffer));

    if (_poll_and_read(fd, response_buffer, sizeof(size_t)) != 0) {
        return 1;
    }

    // calculate proper buffer size
    memcpy((void*)&claimed_packet_size, peakBuffer(response_buffer), sizeof(size_t)); // 8 bytes for packet size
    if (claimed_packet_size > MAX_PACKET_SIZE) {
        // claimed size is too big
        TRACE(("Socket closed, reason: claimed size is too big\n"));
        goto gg_after_receive;
    }

    // we have a proper packet size
    // expand the buffer
    expandBuffer(response_buffer, claimed_packet_size - availableBuffer(response_buffer));

    // poll for data (POLL_IN only)
    if (_poll_and_read(fd, response_buffer, response_buffer->total_size) != 0) {
        return 1;
    }

    close(fd);

    *out = response_buffer;
    return 0;

gg_after_receive:
    destroyBuffer(response_buffer);
    close(fd);
    return 1;
}