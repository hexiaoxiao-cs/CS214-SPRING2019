#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <util.h>
#include <stdint.h>
/*
 *  return:
 *      0: have a response
 *      1: does not have a response
 *
 *  in:
 *      this buffer will be freed by network layer when able
 *
 *  out:
 *      out is not modified when this function returning 1
 */

int send_request(const char* hostname, uint16_t port, buffer* request, buffer** response);
#endif