//
// Created by Zhizhang Deng on 2019-05-01.
//


#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#include <util.h>

#include <string.h>


int _poll_and_write(int fd, buffer* buf) {
    int poll_res;
    ssize_t write_res;
    struct pollfd fds[1];
    size_t write_accum = 0;

    fds[0].fd = fd;
    fds[0].events = POLLOUT;

    while (1) {
        poll_res = poll(fds, 1, 300000);
        if (poll_res > 0) {
            if (fds[0].revents & (POLLERR | POLLHUP)) {
                // either disconnected or have error on socket
                goto gg;
            }
            if (fds[0].revents & POLLNVAL) {
                // cannot close socket when NVAL
                goto gg_withoutclose;
            }
            write_res = write(fd, peakBuffer(buf) + write_accum, getLengthBuffer(buf) - write_accum);
            if (write_res > 0) {
                write_accum += write_res;
                if (write_accum == getLengthBuffer(buf)) {
                    // sent the whole packet
                    break;  // release resources
                }
            } else if (write_res == 0) {
                //client disconnected
                goto gg;
            } else {
                TRACE(("Write error: %s\n", strerror(errno)));
                goto gg;
            }
        } else if (poll_res == 0) {
            // poll time out
            TRACE(("Wait for write timed out\n"));
            goto gg;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg;
        }
    }
    destroyBuffer(buf); // wrote the whole buffer, do not need to keep it
    return 0;

    gg:
    close(fd);
    gg_withoutclose:
    destroyBuffer(buf);
    return 1;
}

int _poll_and_read(int fd, buffer* buf, size_t stop_watermark) {
    int poll_res;
    ssize_t read_res;
    struct pollfd fds[1];

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    if (getLengthBuffer(buf) >= stop_watermark) {
        return 0;   // data is ready already
    }

    // poll for size first
    // POLL_IN only
    while (1) {
        poll_res = poll(fds, 1, 300000);
        if (poll_res > 0) {
            if (fds[0].revents & (POLLERR | POLLHUP)) {
                // either disconnected or have error on socket
                goto gg;
            }
            if (fds[0].revents & POLLNVAL) {
                // cannot close socket when NVAL
                goto gg_withoutclose;
            }
            read_res = read(fd, lastposBuffer(buf), availableBuffer(buf));
            if (read_res > 0) {
                buf->size += read_res;  // increase internal buffer size
                if (getLengthBuffer(buf) >= stop_watermark) {
                    // make sure we have reached the water mark
                    break;
                }
            } else if (read_res == 0) {
                //client disconnected
                goto gg;
            } else {
                //error happened
                TRACE(("Read error: %s\n", strerror(errno)));
                goto gg;
            }
        } else if (poll_res == 0){
            // time out for poll
            TRACE(("Socket timed out\n"));
            goto gg;
        } else {
            // internal error for poll
            TRACE(("Poll error: %s\n", strerror(errno)));
            goto gg;
        }
    }

    return 0;

    gg:
    close(fd);
    gg_withoutclose:
    destroyBuffer(buf);
    return 1;

}
