#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_PACKET_SIZE 1024 * 1024 * 1024 * 5LL      //5 GB maximum packet size

#include "util.h"

/*
 * return val:
 *  0: set out_packet already
 *  1: close connection without responding
 *
 * in_packet:
 *  a pointer to a full packet received by network
 *
 * out_packet:
 *  a pointer pointing to a pointer that should be set to the pointer to desired,
 *  response_packet buffer when return value is 0
 */
int process_packet(buffer* in_packet, buffer** out_packet);

#endif
