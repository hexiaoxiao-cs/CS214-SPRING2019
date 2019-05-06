#include "protocol.h"
#include "util.h"

#include <string.h>
#include <pthread.h>

void _roller_read(buffer* in_packet, void* output, size_t* offset, size_t size) {
    memcpy(output, peakBuffer(in_packet) + (*offset), size);
    *offset += size;
}

#if defined(SERVER_COMPILING)
#include <logic.h>

__thread buffer* in_packet_global;


void free_in_packet() {
    destroyBuffer(in_packet_global);
}

int process_packet(buffer* in_packet, buffer** out_packet) {
    parsed_request_t request;
    in_packet_global = in_packet; // store buffer pointer to thread local storage
    if (parse_request(in_packet, &request) == 1) {
        destroyBuffer(in_packet);   // this has to be destroyed before return
        return 1;   // malformed packet should close connection
    }
    *out_packet = process_logic(&request);
    return 0;
}


int parse_request(buffer* in_packet, parsed_request_t* out) {
    size_t packet_size, offset = 0;
    // we trust network layer checked packet size already
    _roller_read(in_packet, &packet_size, &offset, sizeof(size_t));
    _roller_read(in_packet, &out->op_code, &offset, sizeof(uint8_t));
    if (out->op_code < 0 || out->op_code > 9)
        return 1;   // this is not a valid op_code
    _roller_read(in_packet, &out->project_name_size, &offset, sizeof(size_t));
    if (out->project_name_size > MAX_PACKET_SIZE) {
        // malformed packet
        return 1;
    }
    if (out->project_name_size >= getLengthBuffer(in_packet) - offset) {
        // malformed packet
        return 1;
    }
    out->project_name = peakBuffer(in_packet) + offset;
    offset += out->project_name_size;

    _roller_read(in_packet, &out->is_two_payload, &offset, sizeof(uint8_t));
    if (out->is_two_payload != 0 && out->is_two_payload != 1) {
        // malformed packet
        return 1;
    }
    if (out->is_two_payload) {
        _roller_read(in_packet, &out->files_payload.payload1_size, &offset, sizeof(size_t));
        if (out->files_payload.payload1_size > MAX_PACKET_SIZE) {
            // malformed packet
            return 1;
        }
        if (out->files_payload.payload1_size >= getLengthBuffer(in_packet) - offset) {
            // malformed packet
            // reason: after payload1_size should be either at the end of packet or before the end
            // note: cannot be equal: because there has to be a payload 2 size
            return 1;
        }
        out->files_payload.payload1 = peakBuffer(in_packet) + offset;
        offset += out->files_payload.payload1_size;
        out->files_payload.payload2_size = getLengthBuffer(in_packet) - offset;
        if (out->files_payload.payload2_size == 0)
            out->files_payload.payload2 = NULL;
        else
            out->files_payload.payload2 = peakBuffer(in_packet) + offset;
    } else {
        // only one payload
        out->str_payload.payload_size = getLengthBuffer(in_packet) - offset;
        if (out->str_payload.payload_size == 0)
            out->str_payload.payload = NULL;
        else
            out->str_payload.payload = peakBuffer(in_packet) + offset;
    }
    return 0;
}


buffer* get_output_buffer_for_response(uint16_t status_code, uint8_t is_two_payload) {
    buffer* buf = createBuffer();
    buf->size += sizeof(size_t); // reserve 8 bytes to hold size
    // 42 left
    memcpy(lastposBuffer(buf), (void*)&status_code, sizeof(uint16_t));
    buf->size += sizeof(uint16_t);
    // 40 left
    buf->size += 1; // reserve 1 byte for type of packet
    // 39 left
    if (is_two_payload) {
        buf->data[buf->size - 1] = (uint8_t)1;
        buf->size += sizeof(size_t); // reserve another 8 bytes to hold the size of first payload
        // 31 left
    } else {
        buf->data[buf->size - 1] = (uint8_t)0;
    }
    return buf;
}

void finalize_file_payload1_for_response(buffer *buf) {
    // SIZE[size_t] + STATUS[uint16_t] + TYPE[uint8_t] + PAYLOAD 1 SIZE [size_t] + (PAYLOAD DATA)
    size_t size_of_payload1 = getLengthBuffer(buf) - sizeof(size_t) - sizeof(uint16_t) - sizeof(uint8_t) - sizeof(size_t);
    memcpy(peakBuffer(buf) + sizeof(size_t) + sizeof(uint16_t) + sizeof(uint8_t), (void*)&size_of_payload1, sizeof(size_t));
}



#elif defined(CLIENT_COMPILING)

int parse_response(buffer* in_packet, parsed_response_t* out) {
    size_t packet_size, offset = 0;
    // we trust network layer checked packet size already
    _roller_read(in_packet, &packet_size, &offset, sizeof(size_t));
    _roller_read(in_packet, &out->status_code, &offset, sizeof(uint16_t));
    _roller_read(in_packet, &out->is_two_payload, &offset, sizeof(uint8_t));
    if (out->is_two_payload != 0 && out->is_two_payload != 1) {
        // malformed packet
        return 1;
    }
    if (out->is_two_payload) {
        _roller_read(in_packet, &out->files_payload.payload1_size, &offset, sizeof(size_t));
        if (out->files_payload.payload1_size > MAX_PACKET_SIZE) {
            // malformed packet
            return 1;
        }
        if (out->files_payload.payload1_size >= getLengthBuffer(in_packet) - offset) {
            // malformed packet
            // reason: after payload1_size should be either at the end of packet or before the end
            // note: cannot be equal: because there has to be a payload 2 size
            return 1;
        }
        out->files_payload.payload1 = peakBuffer(in_packet) + offset;
        offset += out->files_payload.payload1_size;
        out->files_payload.payload2_size = getLengthBuffer(in_packet) - offset;
        if (out->files_payload.payload2_size == 0)
            out->files_payload.payload2 = NULL;
        else
            out->files_payload.payload2 = peakBuffer(in_packet) + offset;
    } else {
        // only one payload
        out->str_payload.payload_size = getLengthBuffer(in_packet) - offset;
        if (out->str_payload.payload_size == 0)
            out->str_payload.payload = NULL;
        else
            out->str_payload.payload = peakBuffer(in_packet) + offset;
    }
    return 0;
}

//0-not two payload

buffer* get_output_buffer_for_request(uint8_t op_code, const char* project_name, size_t project_name_size, uint8_t is_two_payload) {
    buffer* buf = createBuffer();
    buf->size += sizeof(size_t);    // reserve 8 bytes to hold size
    // 42 left
    memcpy(lastposBuffer(buf), (void*)&op_code, sizeof(uint8_t));
    buf->size += 1; // op_code
    // 41 left
    memcpy(lastposBuffer(buf), &project_name_size, sizeof(size_t));
    buf->size += sizeof(size_t);    // reserve 8 bytes for project size
    // 33 left
    expandBuffer(buf, project_name_size + 41 - availableBuffer(buf));   // reserve project_name + packet_type + 40 bytes[at least 40 - 8 = 32 bytes are reserved for logical layer]
    // copy project name
    memcpy(lastposBuffer(buf), project_name, project_name_size);
    // claim project_name_size length
    buf->size += project_name_size;
    // claim one more byte to hold packet type
    buf->size += 1;
    if (is_two_payload) {
        buf->data[buf->size - 1] = (uint8_t)1;
        buf->size += sizeof(size_t);    // reserve another 8 bytes to hold the size of first payload
    } else {
        buf->data[buf->size - 1] = (uint8_t)0;
    }
    return buf;
}

void finalize_file_payload1_for_request(buffer *buf) {
    // SIZE[size_t] + OP_CODE[uint8_t] + PROJECT_SIZE[size_t] + PROJECT NAME[PROJECT_SIZE] + TYPE[uint8_t] + PAYLOAD 1 SIZE [size_t] + (PAYLOAD DATA)
    size_t project_name_size, size_of_payload1;
    memcpy(&project_name_size, peakBuffer(buf) + sizeof(size_t) + sizeof(uint8_t), sizeof(size_t));
    size_of_payload1 = getLengthBuffer(buf) - sizeof(size_t) - sizeof(uint8_t) - sizeof(size_t) - project_name_size - sizeof(uint8_t) - sizeof(size_t);
    memcpy(peakBuffer(buf) + sizeof(size_t) + sizeof(uint8_t) + sizeof(size_t) + project_name_size + sizeof(uint8_t), (void*)&size_of_payload1, sizeof(size_t));
}

#endif

void finalize_buffer(buffer *buf) {
    size_t size_of_buffer = getLengthBuffer(buf);
    memcpy(peakBuffer(buf), (void*)&size_of_buffer, sizeof(size_t));
}