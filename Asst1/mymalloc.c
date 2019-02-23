#include "mymalloc.h"   //This can guarantee we are not using malloc internally

#include <stdio.h>  //For debug

#ifdef _WIN32
#define PACK( __declaration__ ) __pragma( pack(push, 1) ) __declaration__ __pragma( pack(pop) )
#else
#define PACK( __declaration__ ) __declaration__ __attribute__((__packed__))
#endif

static char blocks[4096];   //Will be automatically initialized to 0 since it's static

PACK(
	struct header {
		unsigned char is_used : 1;
		unsigned char is_large : 1;
	}
);

PACK(
	struct header_sm {
		unsigned char : 1;
		unsigned char : 1;
		unsigned int size : 6;
	}
);

PACK(
	struct header_lg {
		unsigned char : 1;
		unsigned char : 1;
		unsigned int size : 12;
		unsigned char : 2;
	}
);

void read_header(void* data, char* is_used, char* is_large, int* size) {
	struct header* h = data;
	*is_used = h->is_used;
	*is_large = h->is_large;
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
		h = (char*)current_header + sizeof(struct header_lg) + h_lg->size;
	} else {
		struct header_sm* h_sm = current_header;
		h = (char*)current_header + sizeof(struct header_sm) + h_sm->size;
	}
	if((char*)h > blocks + 4096)
		return NULL;
	else
		return h;
}

void* malloc(int size) {
	return NULL;
}

void free(void* input) {
	void* cur = blocks;
	void* next_blk;
	char is_used, is_large;
	int size, next_size;
	
	//Check if input is in our range
	if(input < blocks || input > blocks + 4096) {
		return;	//input is not in our blocks
	}

	while(cur != NULL) {
		read_header(cur, &is_used, &is_large, &size);
		if((char*)cur + size == input) {
			if(is_used)
				break;	//Only free when it's allocated
			else
				return;	//exit if it is not allocated by us
		}
		cur = get_next_header(cur);
	}
	if(cur == NULL) {
		//Unable to find a block on the designated area
		return;
	}
	//Successfully found a block containing the input data
	next_blk = get_next_header(cur);	//Get the next header
	while(next_blk != NULL) {
		read_header(next_blk, &is_used, &is_large, &next_size);
		if(is_used)
			break;
		else
			size += (next_size + is_large?2:1);	//increase current block's size with next block's size and next block's header size
		next_blk = get_next_header(next_blk);
	}
	write_header(cur, 0, size);	//write new information to the blocks
}