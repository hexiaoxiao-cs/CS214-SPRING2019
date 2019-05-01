#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_PACKET_SIZE 1024 * 1024 * 1024 * 5LL      //5 GB maximum packet size

#include "util.h"
#include <stddef.h>
#include <stdint.h>

/*
 * STRUCTURE NOTE:
 *
 * FOR SERVER:
 *      NETWORK LAYER (INITIATOR) -> PROTOCOL LAYER -> LOGICAL LAYER -> RETURN TO NETWORK LAYER
 *
 * FOR CLIENT:
 *      LOGICAL LAYER (INITIATOR) -> NETWORK LAYER -> RETURN TO LOGICAL LAYER -> PROTOCOL LAYER (TOOL) -> RETURN TO LOGICAL LAYER
 *
 */

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
    uint8_t is_two_payload;
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
 * give control to protocol layer, and generate a network response
 *
 * return val:
 *  0: out_packet has been set
 *  1: close connection without responding
 *
 * in_packet:
 *  a pointer to a full packet received by network, this will be freed by protocol layer when able
 *
 * out_packet:
 *  a pointer pointing to a pointer that should be set to the pointer to desired,
 *  out_packet will be set when return value is 0
 *
 */
int process_packet(buffer* in_packet, buffer** out_packet);

/*
 * API for logic layer to free underlying input packet after finish using parsed_request_t
 */
void free_in_packet();

/*
 * This function need to be called by logical layer after it has set its first payload (if using two payloads response)
 * If called with only one payload's response is UB
 */
void finalize_file_payload1_for_response(buffer* buf);

#elif defined (CLIENT_COMPILING)

/*
 * This function need to be called by logical layer after it has set its first payload in the buffer (if using two payloads response)
 * If called with only one payload's response is UB
 */
void finalize_file_payload1_for_request(buffer* buf);

#endif


/*
 * Utility function to parse a response buffer into parsed_response_t
 *
 * return val:
 *  0: parse success
 *  1: malformed packet
 *
 * SUGGESTION: use stack space to hold out
 */
int parse_response(buffer* in_packet, parsed_response_t* out);

/*
 * Utility function to parse a request buffer into parsed_request_t
 * return val:
 *  0: parse success
 *  1: malformed packet
 *
 * SUGGESTION: use stack space to hold out
 */
int parse_request(buffer* in_packet, parsed_request_t* out);

/*
 * Generate a buffer to be used as a request buffer
 */
buffer* get_output_buffer_for_request(uint8_t op_code, const char* project_name, size_t project_name_size, uint8_t is_two_payload);

/*
 * Generate a buffer to be used as a response buffer
 */
buffer* get_output_buffer_for_response(uint16_t status_code, uint8_t is_two_payload);

/*
 * This function need to be called by logical layer after it has set all of its payload
 */
void finalize_buffer_for(buffer* buf);

#endif
