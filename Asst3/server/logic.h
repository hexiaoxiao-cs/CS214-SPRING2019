#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <stddef.h>

#include "util.h"
#include "protocol.h"

buffer* process_logic(const parsed_request_t* op);

#endif
