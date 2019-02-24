#include "mymalloc.h"   //This can guarantee we are not using malloc internally

#include <stdio.h>  //For debug

#ifdef _WIN32
#define __FUNCTION_NAME__ __FUNCTION__
#define PACK( __declaration__ ) __pragma( pack(push, 1) ) __declaration__ __pragma( pack(pop) )
#else
#define __FUNCTION_NAME__ __func__
#define PACK( __declaration__ ) __declaration__ __attribute__((__packed__))
#endif

static char blocks[4096];   //Will be automatically initialized to 0 since it's static

/*
 * Important: The following blocks only valid under C99 standard (variadic macros for debugging)
 */

//DEBUG TOGGLE
#define DBG


#ifndef DBG
#define DLOG(x) (void)0
#else
#define DLOG(args...) printf("%s[%d]: ", __FUNCTION_NAME__, __LINE__); printf(args); printf("\n"); fflush(stdout)
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

void* get_data_pointer(void* header) {
    char is_used, is_large;
    int size;
    read_header(header, &is_used, &is_large, &size);
    return header + (is_large?2:1);
}

int get_blk_size(void* header) {
    char is_used, is_large;
    int size;
    read_header(header, &is_used, &is_large, &size);
    return size + (is_large?2:1);
}

int adjust_header_size(void* data, int new_size, char is_used) {
    struct header* h = data;
    int blk_size = get_blk_size(data);
    h->is_used = is_used;
    DLOG("Requested to adjust size to: %d", new_size);
    if(!h->is_large) {
        //Originally is a small header
        if(new_size > 64) {
            //Unable to hold the size that is Requesting
            h->is_large = 1;    //Convert to large header
            new_size--; //Reduce one byte from requesting size to accomodate header size change
        }
    } else {
        //Originally is a large header
        if(new_size <= 63) {
            h->is_large = 0;    //Convert to small header
        }
    }
    if(h->is_large) {
        struct header_lg* h_lg = data;
        h_lg->size = new_size;
        DLOG("Actualy adjusted size to: %d", new_size);
        return blk_size - new_size - 2;
    } else {
        struct header_sm* h_sm = data;
        h_sm->size = new_size;
        DLOG("Actually adjusted size to: %d", new_size);
        return blk_size - new_size - 1;
    }
}

void write_new_header(void* data, char is_used, int blk_size) {
    struct header* h = data;
    h->is_used = is_used;
    if(blk_size > 65) {
        //Use a large header
        h->is_large = 1;
        struct header_lg* h_lg = data;
        h_lg->size = blk_size - 2;
    } else {
        //Use a small header
        h->is_large = 0;
        struct header_sm* h_sm = data;
        h_sm->size = blk_size - 1;
    }
}

void* get_previous_header(void* current_header) {
    void* prev = blocks;
    while(prev && get_next_header_position(prev) != current_header) {
        prev = get_next_header_position(prev);
    }
    DLOG("get_previous_header: Returning: %p", prev);
    return prev;
}

void* get_next_header_position(void* current_header) {
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
    void* next_header = NULL;
    char is_used, is_large,Success=0;
    int blk_size, blk_remaining_size;
    //Start to Malloc
    DLOG("Requesting for: %d byte(s)", size);
    //Check if the block is fresh
    read_header(cur,&is_used,&is_large,&blk_size);
    if (is_used == 0 && is_large == 0 && blk_size == 0) { //block is fresh
        if (size > 4095) {
            DLOG("Unable to allocate due to requested size is too large");
            return NULL;
        }
        //Initialize block
        write_new_header(blocks, 0, 4096);
        DLOG("Initializing blocks");
    }
    while (Success == 0) {
        if (cur == NULL) {
            DLOG("Unable to allocate due to no blocks are available");
            return NULL;
        }
        read_header(cur, &is_used, &is_large, &blk_size);
        if (is_used == 0 && blk_size >=size) {
            DLOG("Found blk[ACCPTD]: %p, %d-%d-%d", cur, is_used, is_large, blk_size);
            Success = 1;
        }
        else
        {
            DLOG("Found blk[DISCARD]: %p, %d-%d-%d", cur, is_used, is_large, blk_size);
            cur = get_next_header_position(cur);
        }
    }
    blk_remaining_size = adjust_header_size(cur, size, 1);
    next_header = get_next_header_position(cur);
    if(next_header == NULL) {
        DLOG("Successfully allocated to: %p", get_data_pointer(cur));
        return get_data_pointer(cur);
    }
    write_new_header(next_header, 0, blk_remaining_size);
    DLOG("Successfully allocated to: %p", get_data_pointer(cur));
    return get_data_pointer(cur);
}

void myfree(void* input) {
    void* cur = blocks;
    void* next_blk, *prev_blk;
    char is_used, is_large, is_large_next, is_large_prev;
    int size, next_size, prev_size;

    //Check if input is in our range
    if((char*)input < blocks || (char*)input > &blocks[4095]) {
        DLOG("Unable to free, input is not in blocks");
        return; //input is not in our blocks
    }

    //Try to find the block that has the input as data
    while(cur != NULL) {
        read_header(cur, &is_used, &is_large, &size);
        //Compare with input after skipping the header
        if(get_data_pointer(cur) == input) {
            if(is_used)
                break;  //Only free when it's allocated
            else {
                DLOG("Unable to free, input is in blocks but is not used");
                return; //exit if it is not allocated by us
            }
        }
        cur = get_next_header_position(cur);
    }

    if(cur == NULL) {
        //Unable to find a block on the designated area
        DLOG("Unable to free, unable to find data pointers in blocks that matches input");
        return;
    }

    DLOG("Free Found target blk: %p, %d-%d-%d", cur, is_used, is_large, size);

    //Successfully found a block containing the input data

    next_blk = get_next_header_position(cur);   //Get the next header
    if(next_blk) {
        read_header(next_blk, &is_used, &is_large_next, &next_size);
        DLOG("Free Found Nxt blk: %p, %d-%d-%d", next_blk, is_used, is_large_next, next_size);
        if(!is_used)
            size += get_blk_size(next_blk); //increase current block's size with next block's size and next block's header size
    }
    adjust_header_size(cur, size, 0);   //Adjust current block's size

    prev_blk = get_previous_header(cur);
    if(prev_blk) {
        read_header(prev_blk, &is_used, &is_large_prev, &prev_size);
        if(!is_used) {
            //Squash previous empty blks with current blk [possibly with one after current blk as well]
            int prev_new_size = prev_size + get_blk_size(cur);
            DLOG("Squashing %p with previous blk: %p, previous orig size: %d, previous new size: %d", cur, prev_blk, prev_size , prev_new_size);
            adjust_header_size(prev_blk, prev_new_size, 0);
        }
    }
    DLOG("Free succeed on: %p", input);
}
