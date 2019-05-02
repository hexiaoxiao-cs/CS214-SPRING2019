#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

int writeFile(const char *file_path, char *data, size_t size) {
    int handler = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (handler < 0) {
        //Unable to open specific files
        printf("Unable to open file %s\n", file_path);
    }
    //Blocking and writeAll
    size_t tmp;
    ssize_t ret;
    tmp = 0;
    while (tmp < size) {
        ret = write(handler, data + tmp, size - tmp);
        if (ret < 0) {
            return -1;
        } else if (ret == 0) {
            return -1;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    close(handler);    //Close file
    return 0;
}

int isDir(const char *name) {
    DIR *directory = opendir(name);

    if (errno !=0) {
        return -1;
    }
    return 0;
}


int readFile(char *filename, char** buffer,size_t *size){
    int fno=open(filename,O_RDONLY);
    if(fno<0){return -1;}
    //readAll
    size_t tmp;
    ssize_t ret;
    size_t file_size= (size_t) lseek(fno, 0,SEEK_END);
    lseek(fno,0,SEEK_SET);
    char *data = (char*) malloc(file_size+1);
    data[file_size]=0;
    tmp=0;
    while(tmp<file_size){
        ret = read(fno,data+tmp,file_size-tmp);
        if(ret<0){return -1;}
        else  if(ret==0){break;}
        else{
            tmp+=ret;
        }
    }
    *buffer=data;
    *size=file_size;
}


void dbg_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

buffer *createBuffer() {
    buffer *space = calloc(1, sizeof(buffer));
    space->data = calloc(1, 50 + 1);  //50 bytes, last bytes for NULL-terminator
    space->total_size = 50;
    return space;
}

void destroyBuffer(buffer *space) {
    free(space->data);
    free(space);
}
void destroyBufferWithoutFree(buffer *space) {
    free(space);
}

void expandBuffer(buffer *space, size_t size) {
    if (size <= 0)
        return;
    space->total_size = space->total_size + size;
    void *tmp = realloc(space->data, space->total_size + 1);
    if(tmp == NULL) {
        printf("Unable to allocate %ld bytes of memory\n", space->total_size + 1);
        exit(1);
    }
    space->data = tmp;
}

//void appendExpandable(buffer *space, char c) {
//    space->data[space->size++] = c;
//    if (space->size == space->total_size) {
//        if (space->size <= 1024)
//            expandExpandable(space, 10);  //expand 10 bytes each time for small memory
//        else
//            expandExpandable(space, space->size); //double the size each time for large memory
//    }
//    space->data[space->size] = 0;   //set the next byte to be 0
//}

/*
 * We do not need a remove buffer here
 * because our protocol definition is one packet only per connection, so there is no need to reuse connection to process multiple packets
 */

char* peakBuffer(buffer* ptr){
    return ptr->data;
}

size_t availableBuffer(buffer* ptr) {
    return ptr->total_size - ptr->size;
}

char* lastposBuffer(buffer* ptr) {
    return ptr->data + ptr->size;
}

void copyoutBuffer(buffer *ptr, char* data, size_t size) {
    memcpy(data, ptr->data, size);
}

size_t getLengthBuffer(buffer *ptr){
    return ptr->size;
}

void appendSequenceBuffer(buffer *space, const char *sequence, size_t sequence_size) {
    size_t available = space->total_size - space->size; //precalculate available size
    if (available <= sequence_size) {
        if (space->size + sequence_size > 1024) {
            expandBuffer(space, (sequence_size - available) + 4096000); //expand 4M each time for large memory
        } else {
            expandBuffer(space, sequence_size - available + 10);  //expand 10 bytes each time for small memory
        }
    }

    memcpy(space->data + space->size, sequence, sequence_size);
    space->size += sequence_size;
    space->data[space->size] = 0; //set the next byte to be 0
}

buffer* duplicateBuffer(buffer* space) {
    buffer* new_buffer = (buffer*)malloc(sizeof(buffer));
    new_buffer->size = space->size;
    new_buffer->total_size = space->total_size;
    new_buffer->data = (char*)malloc(new_buffer->total_size + 1);   //add one here because we need space for the null terminator
    memcpy(new_buffer->data, space->data, new_buffer->size + 1);    // add one here because buffer is guaranteed null terminated
    return new_buffer;
}

void zeroUnusedBuffer(buffer *space) {
    memset(space->data + space->size, 0, space->total_size - space->size);  //zero out unused space
}
