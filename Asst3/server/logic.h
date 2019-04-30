#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <stddef.h>

#include "util.h"

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
} operation ;

buffer* process_logic(const operation* op);

#endif
