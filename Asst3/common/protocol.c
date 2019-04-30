#include "protocol.h"
#include "util.h"

#include <string.h>
#include <pthread.h>

#if defined(SERVER_COMPILING)
#include <logic.h>

__thread buffer* in_packet_global;


void free_in_packet() {
    destroyBuffer(in_packet_global);
}

int process_packet(buffer* in_packet, buffer** out_packet) {
    in_packet_global = in_packet;
    parse_request(in_packet);
}

void finalize_file_payload1_for_response(buffer *buf) {
    size_t size_of_payload1 = getLengthBuffer(buf) - sizeof(size_t) - sizeof(uint8_t) - 1 - sizeof(size_t);
    memcpy(peakBuffer(buf) + sizeof(size_t) + sizeof(uint8_t) + 1, (void*)&size_of_payload1, sizeof(size_t));
}

void finalize_buffer_for_response(buffer *buf) {
    size_t size_of_buffer = getLengthBuffer(buf);
    memcpy(peakBuffer(buf), (void*)&size_of_buffer, sizeof(size_t));
}

#elif defined(CLIENT_COMPILING)

void finalize_file_payload1_for_request(buffer *buf) {

}

void finalize_buffer_for_request(buffer *buf) {

}

#endif

parsed_response_t parse_response(buffer* in_packet) {

}

parsed_request_t parse_request(buffer* in_packet) {

}

buffer* get_output_buffer_for_request(uint8_t op_code, const char* project_name, uint8_t project_name_size) {

}


buffer* get_output_buffer_for_response(uint16_t status_code, uint8_t is_two_payload) {
    buffer* buf = createBuffer();
    buf->size += sizeof(size_t); // reserve 8 bytes to hold size
    // initial 42 bytes are enough to hold one byte
    memcpy(peakBuffer(buf), (void*)&status_code, sizeof(uint8_t));
    buf->size += sizeof(uint8_t);
    buf->size += 1; // reserve 1 byte for type of packet
    if (is_two_payload) {
        buf->data[buf->size - 1] = (uint8_t)1;
        buf->size += sizeof(size_t); // reserve another 8 bytes to hold the size of first payload
    } else {
        buf->data[buf->size - 1] = (uint8_t)2;
    }
    return buf;
}
