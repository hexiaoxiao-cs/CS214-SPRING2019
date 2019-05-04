#ifndef LOGIC_H
#define LOGIC_H

#include <stdint.h>
#include <stddef.h>

#include <util.h>
#include "protocol.h"
#include "ds.h"

buffer* process_logic(parsed_request_t* op);

#endif
