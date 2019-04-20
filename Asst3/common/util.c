#include <stdlib.h>
#include<string.h>
#include "util.h"
expandable *createExpandable() {
    expandable *space = calloc(1, sizeof(expandable));
    space->data = calloc(1, 50 + 1);  //50 bytes, last bytes for NULL-terminator
    space->total_size = 50;
    return space;
}

void destroyExpandable(expandable *space) {
    free(space->data);
    free(space);
}
void destroyExpandableWithoutFree(expandable *space) {
    free(space);
}

void expandExpandable(expandable *space, size_t size) {
    space->total_size = space->total_size + size;
    void *tmp = realloc(space->data, space->total_size + 1);
    if(tmp == NULL) {
        printf("Unable to allocate %ld bytes of memory\n", space->total_size + 1);
        exit(1);
    }
    space->data = tmp;
}

//void appendExpandable(expandable *space, char c) {
//    space->data[space->size++] = c;
//    if (space->size == space->total_size) {
//        if (space->size <= 1024)
//            expandExpandable(space, 10);  //expand 10 bytes each time for small memory
//        else
//            expandExpandable(space, space->size); //double the size each time for large memory
//    }
//    space->data[space->size] = 0;   //set the next byte to be 0
//}

char* drainExpandable(expandable* ptr, size_t size)
{
    char* toReturn=(char*) malloc((size+1)*sizeof(char));
    toReturn=strncpy(toReturn, ptr->data,size);
    ptr->data+=size;
    ptr->size-=size;
    return toReturn;
}

char* peakExpandable(expandable* ptr,size_t size){
    char* toReturn=(char*) malloc((size+1)*sizeof(char));
    toReturn=strncpy(toReturn, ptr->data,size);
    return toReturn;
}

size_t getExpandableLength(expandable *ptr){
    return ptr->size;
}

void appendSequenceExpandable(expandable *space, const char *sequence, size_t sequence_size) {
    size_t available = space->total_size - space->size; //precalculate available size
    if (available <= sequence_size) {
        if (space->size + sequence_size > 1024) {
            expandExpandable(space, (sequence_size - available) + 4096000); //expand 4M each time for large memory
        } else {
            expandExpandable(space, sequence_size - available + 10);  //expand 10 bytes each time for small memory
        }

    }

    memcpy(space->data + space->size, sequence, sequence_size);
    space->size += sequence_size;
    space->data[space->size] = 0; //set the next byte to be 0
}

void zeroUnusedExpandable(expandable *space) {
    memset(space->data + space->size, 0, space->total_size - space->size);  //zero out unused space
}