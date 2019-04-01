#include "ds.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

expandable *createExpandable() {
    expandable *space = calloc(1, sizeof(expandable));
    space->data = calloc(1, 5 + 1);  //5 bytes + 1 null terminator(reserved for codes)
    space->total_size = 5;
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
	panic("Unable to allocate enough memory");
    }
    space->data = tmp;
}

void appendExpandable(expandable *space, char c) {
    space->data[space->size++] = c;
    if (space->size == space->total_size) {
        if (space->size <= 1024)
            expandExpandable(space, 10);
        else
            expandExpandable(space, space->size);
    }
    space->data[space->size] = 0;   //set the next byte to be 0
}

void appendSequenceExpandable(expandable *space, const char *sequence, size_t sequence_size) {
    size_t available = space->total_size - space->size;
    if (available < sequence_size) {
        if (space->size + sequence_size > 1024) {
            expandExpandable(space, (sequence_size - available) + 4096000);
        } else {
            expandExpandable(space, sequence_size - available + 10);
        }

    }

    memcpy(space->data + space->size, sequence, sequence_size);
    space->size += sequence_size;
    space->data[space->size] = 0;
//	for(int i=0;i < sequence_size;i++) {
//		appendExpandable(space, sequence[i]);
//	}
}

void zeroUnusedExpandable(expandable *space) {
    memset(space->data + space->size, 0, space->total_size - space->size);
}

expandablePtr *createExpandablePtr() {
    expandablePtr *space = calloc(1, sizeof(expandablePtr));
    space->data = calloc(1, sizeof(void *) * 10);  //10 void pointers
    space->total_size = 10; //10 void pointers
    return space;
}

void destroyExpandablePtr(expandablePtr *space) {
    free(space->data);
    free(space);
}

void destroyExpandablePtrWithoutFree(expandablePtr *space) {
    free(space);
}

void expandExpandablePtr(expandablePtr *space) {
    space->total_size = space->total_size + 10;
    space->data = realloc(space->data, space->total_size * sizeof(void *));  //null terminator (reserved for codes)
}

void appendExpandablePtr(expandablePtr *space, const void *c) {
    space->data[space->size++] = (void *) c;
    if (space->size == space->total_size) {
        expandExpandablePtr(space);
    }
}

void heapify(MinHeap *heap, int index) {
    int left = index * 2 + 1;
    int right = index * 2 + 2;
    node *temp;
    if (left < heap->size && heap->array[left]->count < heap->array[index]->count) {
        //swap the left to the index
        temp = heap->array[index];
        heap->array[index] = heap->array[left];
        heap->array[left] = temp;
        heapify(heap, left);
    }
    if (right < heap->size && heap->array[right]->count < heap->array[index]->count) {
        //swap the right to the index
        temp = heap->array[index];
        heap->array[index] = heap->array[right];
        heap->array[right] = temp;
        heapify(heap, right);
    }
    return;
}

node *getMinNodeHeap(MinHeap *heap) {
    node *toReturn = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];
    heap->size--;
    heapify(heap, 0);
    return toReturn;
}

void insertNode(MinHeap *heap, node *toInsert) {
    int curr = heap->size;
    while (curr != 0 && toInsert->count < heap->array[(curr - 1) / 2]->count) {
        heap->array[curr] = heap->array[(curr - 1) / 2];
        curr = (curr - 1) / 2;
    }
    heap->array[curr] = toInsert;
    heap->size++;
    return;
}

MinHeap *initMinHeap(node **nodearray, int many) {
    MinHeap *toReturn = (MinHeap *) malloc(sizeof(MinHeap));
    toReturn->size = many;
    toReturn->array = nodearray;
    return toReturn;
}

void
TraverseTreePrefix(expandable **codes, expandable **words, char *curr, int *nowcode, int *nowword, node *currnode) {
    expandable *stuff;
    //At the edge which means that must be a value node
    if (currnode->left == NULL && currnode->right == NULL) {

        //printf("%s\n", currnode->data->data);
        //printf("%d\n",currnode->count);
        curr[*nowcode] = '\0';
        stuff = createExpandable();
        appendSequenceExpandable(stuff, curr, (*nowcode));
        if (words != NULL || codes != NULL) {
            words[*nowword] = currnode->data;
            codes[*nowword] = stuff;
        }
        currnode->codes = stuff;
        //printf("%s\n", codes[*nowword]);
        (*nowword)++;
        return;
    }
    //Left Node
    {
        //Begin Accessing

        curr[*nowcode] = '0';
        (*nowcode)++;
        TraverseTreePrefix(codes, words, curr, nowcode, nowword, currnode->left);
        //Finished Accessing--Cleaning
        (*nowcode)--;
        curr[*nowcode] = '\0';
    }
    //Right Node
    {
        //Begin Accessing

        curr[*nowcode] = '1';
        (*nowcode)++;
        TraverseTreePrefix(codes, words, curr, nowcode, nowword, currnode->right);
        //Finished Accessing--Cleaning
        (*nowcode)--;
        curr[*nowcode] = '\0';
    }
    return;
}
