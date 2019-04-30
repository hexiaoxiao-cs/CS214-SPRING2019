#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_PACKET_SIZE 1024 * 1024 * 1024 * 5LL      //5 GB maximum packet size

#include "util.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t payload1_size;
    const char* payload1;
    size_t payload2_size;
    const char* payload2;
} files_payload_t ;

typedef struct {
    size_t payload_size;
    const char* payload;
} str_payload_t;

typedef struct {
    uint8_t op_code;
    size_t project_name_size;
    const char* project_name;
    union {
        str_payload_t str_payload;
        files_payload_t files_payload;
    };
} parsed_request_t ;

typedef struct {
    uint16_t status_code;
    uint8_t is_two_payload;
    union {
        str_payload_t str_payload;
        files_payload_t files_payload;
    };
} parsed_response_t;

#if defined(SERVER_COMPILING)
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

void free_in_packet();
void finalize_file_payload1_for_response(buffer* buf);
void finalize_buffer_for_response(buffer* buf);

#elif defined (CLIENT_COMPILING)
void finalize_file_payload1_for_request(buffer* buf);
void finalize_buffer_for_request(buffer* buf);
#endif

parsed_response_t parse_response(buffer* in_packet);
parsed_request_t parse_request(buffer* in_packet);
buffer* get_output_buffer_for_request(uint8_t op_code, const char* project_name, uint8_t project_name_size);
buffer* get_output_buffer_for_response(uint16_t status_code, uint8_t is_two_payload);


#endif
