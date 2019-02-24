#include "mymalloc.h"   //This can guarantee we are not using malloc internally

#include <stdio.h>  //For debug

#ifdef _WIN32
#define PACK( __declaration__ ) __pragma( pack(push, 1) ) __declaration__ __pragma( pack(pop) )
#else
#define PACK( __declaration__ ) __declaration__ __attribute__((__packed__))
#endif

static char blocks[4096];   //Will be automatically initialized to 0 since it's static

#define DBG

#ifndef DBG
#define DLOG(x)
#define DLOG1INT(x, y)
#else
#define DLOG(x) printf("L: %d, Info: %s\n", __LINE__, x); fflush(stdout)
#define DLOG1INT(x, y) printf("L: %d, Info: %s (%d)\n", __LINE__, x, y); fflush(stdout)
#endif

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
	} else {
		h->is_large = 0;
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
		h = (struct header*)((char*)current_header + sizeof(struct header_lg) + h_lg->size);
	} else {
		struct header_sm* h_sm = current_header;
		h = (struct header*)((char*)current_header + sizeof(struct header_sm) + h_sm->size);
	}
	if((char*)h > &blocks[4095])
		return NULL;
	else
		return h;
}

void* mymalloc(int size) {
	void* cur = blocks;
	void* next = NULL;
	char is_used, is_large,Success=0;
	int blk_size;
	//Start to Malloc
	DLOG1INT("Requesting for: ", size);
	//Check if the block is fresh
	read_header(cur,&is_used,&is_large,&blk_size);
	if (is_used == 0 && is_large == 0 && blk_size == 0) { //block is fresh
		if (size > 4095) {
			DLOG("Unable to allocate due to requested size is too large");
			return NULL;
		} //The block size is larger than current memory size
		write_header(cur, 1, size);
		next = get_next_header(cur);
		write_header(next, 0, 4096 - (next - cur)-(((4096-(next-cur))>65)?2:1));//write next blk
		DLOG1INT("BLKS: ", blocks);
		DLOG1INT("Sizeof(lg): ", sizeof(struct header_lg));
		DLOG1INT("Sizeof(sm): ", sizeof(struct header_sm));
		DLOG1INT("Successfully allocated[First Allocation] to: ", next - size);
		return next - size; //return addr
	}
	while (Success == 0) {
		if (cur == NULL) {
			DLOG("Unable to allocate due to no blocks are available");
			return NULL;
		}
		read_header(cur, &is_used, &is_large, &blk_size);
		if (is_used == 0 && blk_size >=size) {
			printf("Found blk[ACCPTD]: %d, %d-%d-%d\n", cur, is_used, is_large, blk_size);
			fflush(stdout);
			Success = 1;
		}
		else
		{
			printf("Found blk[DISCARD]: %d, %d-%d-%d\n", cur, is_used, is_large, blk_size);
			fflush(stdout);
			cur = get_next_header(cur);
		}
	}
	write_header(cur, 1, size);
	DLOG1INT("Written in size: ", size);
	next = get_next_header(cur);
	if(next==NULL) {
		DLOG1INT("Successfully allocated to: ", cur + ((size > 64)?2:1));
		return cur + ((size > 64)?2:1);
	}
	DLOG1INT("Located next blk: ", next);
	write_header(next, 0, blk_size - size - (size > 64 ? 2 : 1));
	DLOG1INT("Successfully allocated to: ", next - size);
	return next-size;
}

void myfree(void* input) {
	void* cur = blocks;
	void* next_blk;
	char is_used, is_large, is_large_next;
	int size, next_size;

	//Check if input is in our range
	if((char*)input < blocks || (char*)input > &blocks[4095]) {
		DLOG("Unable to free, input is not in blocks");
		return;	//input is not in our blocks
	}

	//Try to find the block that has the input as data
	while(cur != NULL) {
		read_header(cur, &is_used, &is_large, &size);
		//Compare with input after skipping the header
		if((char*)cur + (is_large?2:1) == input) {
			if(is_used)
				break;	//Only free when it's allocated
			else {
				DLOG("Unable to free, input is in blocks but is not used");
				return;	//exit if it is not allocated by us
			}
		}
		cur = get_next_header(cur);
	}

	if(cur == NULL) {
		//Unable to find a block on the designated area
		DLOG("Unable to free, unable to find data pointers in blocks that matches input");
		return;
	}

	printf("Free Found target blk: %d, %d-%d-%d\n", cur, is_used, is_large, size);
	fflush(stdout);
	//Successfully found a block containing the input data
	next_blk = get_next_header(cur);	//Get the next header
	while(next_blk != NULL) {
		read_header(next_blk, &is_used, &is_large_next, &next_size);
		printf("Free Found Nxt blk: %d, %d-%d-%d\n", next_blk, is_used, is_large_next, next_size);
		fflush(stdout);
		if(is_used)
			break;
		else
			size += (next_size + (is_large_next?2:1));	//increase current block's size with next block's size and next block's header size
		next_blk = get_next_header(next_blk);
	}
	DLOG1INT("Free writing size: ", size - (!is_large && size >= 65 ? 1 : 0));
	write_header(cur, 0, size - (!is_large && size >= 65 ? 1 : 0));	//write new information to the blocks
	DLOG1INT("Free succeed on: ", input);
	//TODO: sequencial free from left to right will not squash empty spaces. 
}
