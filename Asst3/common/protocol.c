#include "protocol.h"
#include "util.h"
#include "logic.h"

#include <string.h>
#include <pthread.h>

__thread buffer* in_packet;

int process_packet(buffer* in_packet, buffer** out_packet) {

}

void free_in_packet() {
    destroyBuffer(in_packet);
}

buffer* get_output_buffer(uint8_t status_code, uint8_t is_two_payload) {
    buffer* buf = createBuffer();
    buf->size += 8; // reserve 8 bytes to hold size
    // initial 42 bytes are enough to hold one byte
    memcpy(peakBuffer(buf), (void*)&status_code, sizeof(uint8_t));
    buf->size += sizeof(uint8_t);
    buf->size += 1; // reserve 1 byte for type of packet
    if (is_two_payload) {
        buf->data[buf->size - 1] = (uint8_t)1;
        buf->size += 8; // reserve another 8 bytes to hold the size of first payload
    } else {
        buf->data[buf->size - 1] = (uint8_t)2;
    }
    return buf;
}

void finalize_payload1(buffer* buf) {
    
}
