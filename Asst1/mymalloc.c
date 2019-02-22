#include "mymalloc.h"   //This can guarantee we are not using malloc internally

#include <stdio.h>  //For debug

static char blocks[4096];   //Will be automatically initialized to 0 since it's static

struct  __attribute__((__packed__)) header {
	unsigned char isUsed : 1;
	unsigned char isLarge : 1;
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

void readHeader(void* data, char* isUsed, int* size) {

}

void writeHeader(void* data, char isUsed, int size) {

}

void* getNextHeader(void* currentHeader) {
	return NULL;
}

void* malloc(int size) {
	return NULL;
}

void free(void* input) {

}