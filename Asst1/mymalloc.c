#include "mymalloc.h"   //This can guarantee we are not using malloc internally

#include <stdio.h>  //For debug

static char blocks[4096];   //Will be automatically initialized to 0 since it's static

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