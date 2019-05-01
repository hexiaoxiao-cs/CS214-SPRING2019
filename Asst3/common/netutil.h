//
// Created by Zhizhang Deng on 2019-05-01.
//

#ifndef WTF_NETUTIL_H
#define WTF_NETUTIL_H

#include <util.h>

/*
 * Perform poll and append result into the buffer
 * This function will exit if we have an error during poll/read or buffer length >= stop_watermark
 *
 * return val:
 *  0: success
 *  1: error happened during poll/read, buf has been freed and connection has been closed
 * NOTE: buf has to have enough room to hold incoming traffic!
 * NOTE: this function will leave connection open if returning 0
 */
int _poll_and_read(int fd, buffer* buf, size_t stop_watermark);

/*
 * Perform poll and write result into socket from buffer
 * This function will exit if we have an error during poll/write or finish writing the whole buffer
 *
 * return val:
 *  0: success, buf has been freed
 *  1: error happened during poll/write, buf has been freed and connection has been closed
 * NOTE: buf has to have enough room to hold incoming traffic!
 * NOTE: this function will leave connection open if returning 0
 */
int _poll_and_write(int fd, buffer* buf);

#endif //WTF_NETUTIL_H
