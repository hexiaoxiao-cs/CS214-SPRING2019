#include "ds.h"
#include <stdlib.h>
#include <string.h>

expandable* createExpandable() {
	expandable* space = calloc(1, sizeof(expandable));
	space->data = calloc(1, 50 + 1);  //50 bytes + 1 null terminator(reserved for codes)
	space->total_size = 50;
	return space;
}

void destroyExpandable(expandable* space) {
	free(space->data);
	free(space);
}

void destroyExpandableWithoutFree(expandable* space) {
	free(space);
}

void expandExpandable(expandable* space, int size) {
	space->total_size = space->total_size + size;
	space->data = realloc(space->data, space->total_size + 1);  //null terminator (reserved for codes)
	space->data[space->total_size] = 0; //set it to null
}

void appendExpandable(expandable* space, char c) {
	space->data[space->size++] = c;
	if(space->size == space->total_size) {
		expandExpandable(space, 50);
	}
}

void appendSequenceExpandable(expandable* space, const char* sequence, int sequence_size) {
	int available = space->total_size - space->size;
	if(available < sequence_size) {
		expandExpandable(space, sequence_size - available + 50);
	}

	memcpy(space->data + space->size, sequence, sequence_size);
	space->size += sequence_size;
	space->data[space->size] = 0;
//	//TODO: performance can be improved in here
//	for(int i=0;i < sequence_size;i++) {
//		appendExpandable(space, sequence[i]);
//	}
}

void zeroUnusedExpandable(expandable* space) {
	memset(space->data + space->size, 0, space->total_size - space->size);
}

expandablePtr* createExpandablePtr() {
	expandablePtr* space = calloc(1, sizeof(expandablePtr));
	space->data = calloc(1, sizeof(void*)*50);  //50 void pointers
	space->total_size = 50; //50 void pointers
	return space;
}

void destroyExpandablePtr(expandablePtr* space) {
	free(space->data);
	free(space);
}

void destroyExpandablePtrWithoutFree(expandablePtr* space) {
	free(space);
}

void expandExpandablePtr(expandablePtr* space) {
	space->total_size = space->total_size + 50;
	space->data = realloc(space->data, space->total_size *sizeof(void*));  //null terminator (reserved for codes)
}

void appendExpandablePtr(expandablePtr* space, const void* c) {
	space->data[space->size++] = c;
	if(space->size == space->total_size) {
		expandExpandablePtr(space);
	}
}

void appendSequenceExpandablePtr(expandablePtr* space, void* const * sequence, int sequence_size) {
	int i;
	//TODO: performance can be improved in here
	for(i=0;i < sequence_size;i++) {
		appendExpandablePtr(space, sequence[i]);
	}
}

void zeroUnusedExpandablePtr(expandablePtr* space) {
	memset(space->data + space->size, 0, (space->total_size - space->size)*sizeof(void*));
}


void heapify(MinHeap *heap, int index)
{
	int left = index * 2 + 1;
	int right = index * 2 + 2;
	node* temp;
	if (left<heap->size && heap->array[left]->count < heap->array[index]->count)
	{
		//swap the left to the index
		temp = heap->array[index];
		heap->array[index] = heap->array[left];
		heap->array[left] = temp;
		heapify(heap, left);
	}
	if (right<heap->size && heap->array[right]->count < heap->array[index]->count)
	{
		//swap the right to the index
		temp = heap->array[index];
		heap->array[index] = heap->array[right];
		heap->array[right] = temp;
		heapify(heap, right);
	}
	return;
}

node* getMinNodeHeap(MinHeap *heap)
{
	node *toReturn = heap->array[0];
	heap->array[0] = heap->array[heap->size - 1];
	heap->size--;
	heapify(heap, 0);
	return toReturn;
}

void insertNode(MinHeap *heap, node *toInsert)
{
	int curr = heap->size;
	while (curr != 0 && toInsert->count < heap->array[(curr - 1) / 2]->count)
	{
		heap->array[curr] = heap->array[(curr - 1) / 2];
		curr = (curr - 1) / 2;
	}
	heap->array[curr] = toInsert;
	heap->size++;
	return;
}

MinHeap* initMinHeap(node** nodearray, int many)
{
	MinHeap *toReturn = (MinHeap*)malloc(sizeof(MinHeap));
	toReturn->size = many;
	toReturn->array = nodearray;
	return toReturn;
}

/* Creating Node Array
* Required:
* Inputs:
* Char* contents
* int* counts
* int many how many entries
* Sequence: 0 -> inf => least freq -> most freq
* Outputs:
* node* head pointer to head
* Xiaoxiao He 3/12/2019
*/
// node** createNodeArray(expandable** contents, int* counts, int many)
// {
// 	node** array = (node**)malloc(sizeof(node*)*many);
// 	int i = 0;
// 	node* temp;
// 	size = many;
// 	for (i = 0; i<many; i++)
// 	{
// 		temp = (node*)malloc(sizeof(node));
// 		temp->count = counts[i];
// 		temp->data = contents[i];
// 		temp->left = temp->right = NULL;
// 		array[i] = temp;
// 	}
// 	return array;
// }

// void TraverseTreePrefixInternally(char* curr, int *nowcode, int* nowword, node* currnode)
// {
// 	expandable *stuff;
// 	//At the edge which means that must be a value node
// 	if (currnode->left == NULL &&  currnode->right == NULL) {
// 		//printf("%s\n", currnode->data->data);
// 		curr[*nowcode] = '\0';
// 		stuff=createExpandable();
// 		stuff = appendSequenceExpandable(stuff,curr,(*nowcode));
// 		currnode->codes = stuff;
// 		//printf("%s\n", codes[*nowword]);
// 		(*nowword)++;
// 		return;
// 	}
// 	//Left Node
// 	{
// 		//Begin Accessing
//
// 		curr[*nowcode] = '0';
// 		(*nowcode)++;
// 		TraverseTreePrefixInternally(curr, nowcode, nowword, currnode->left);
// 		//Finished Accessing--Cleaning
// 		(*nowcode)--;
// 		curr[*nowcode] = '\0';
// 	}
// 	//Right Node
// 	{
// 		//Begin Accessing
//
// 		curr[*nowcode] = '1';
// 		(*nowcode)++;
// 		TraverseTreePrefixInternally( curr, nowcode, nowword, currnode->right);
// 		//Finished Accessing--Cleaning
// 		(*nowcode)--;
// 		curr[*nowcode] = '\0';
// 	}
// 	return;
// }
#include <stdio.h>

void TraverseTreePrefix(expandable** codes, expandable **words, char* curr, int *nowcode, int* nowword, node* currnode)
{
	expandable *stuff;
	//At the edge which means that must be a value node
	if (currnode->left == NULL &&  currnode->right == NULL) {

		//printf("%s\n", currnode->data->data);
		//printf("%d\n",currnode->count);
		curr[*nowcode] = '\0';
		stuff = createExpandable();
		appendSequenceExpandable(stuff,curr,(*nowcode));
		if(words!=NULL||codes!=NULL){
		words[*nowword] = currnode->data;
		codes[*nowword] = stuff;}
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
