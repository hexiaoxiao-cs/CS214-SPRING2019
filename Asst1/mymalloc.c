#include "mymalloc.h"   //This can guarantee we are not using malloc internally

#include <stdio.h>  //For debug

static char blocks[4096];   //Will be automatically initialized to 0 since it's static

struct  __attribute__((__packed__)) header {
	unsigned char is_used : 1;
	unsigned char is_large : 1;
};

struct __attribute__((__packed__)) header_sm {
	unsigned char : 1;
	unsigned char : 1;
	unsigned int size : 6;
};

struct __attribute__((__packed__)) header_lg {
	unsigned char : 1;
	unsigned char : 1;
	unsigned int size : 12;
	unsigned char : 2;
};

void read_header(void* data, char* is_used, int* size) {
	struct header* h = data;
	*is_used = h->is_used;
	if(h->is_large) {
		struct header_lg* h_lg = data;
		*size = h_lg->size;
	} else {
		struct header_sm* h_sm = data;
		*size = h_sm->size;
	}
}

void write_header(void* data, char is_used, int size) {
	struct header* h = data;
	h->is_used = is_used;
	if(size > 64) {
		h->is_large = 1;
	}
	if(h->is_large) {
		struct header_lg* h_lg = data;
		if(size != -1)
			h_lg->size = size;
	} else {
		struct header_sm* h_sm = data;
		if(size != -1)
			h_sm->size = size;
	}
}

void* get_next_header(void* current_header) {
 	struct header* h = current_header;
	if(h->is_large) {
		struct header_lg* h_lg = current_header;
		return current_header + sizeof(struct header_lg) + h_lg->size;
	} else {
		struct header_sm* h_sm = current_header;
		return current_header + sizeof(struct header_sm) + h_sm->size;
	}
}

void* malloc(int size) {
	return NULL;
}

void free(void* input) {

}