#include<stdio.h>
#include<stdlib.h>
typedef struct node{
	int count;
	struct node *left,*right;
	char* data;
}node;
typedef struct MinHeap {
    int size;
    struct node** array;
}Minheap;

void heapify(MinHeap *heap, int index)
{
	int left = index*2+1;
	int right = index*2+2;
	node* temp;
	if(left<heap->size && heap->array[left]-> count < heap -> array[index]->count )
	{
		//swap the left to the index
		temp=heap->array[index];
		heap->array[index]=heap->array[left];
		heap->array[left]=temp;
		heapify(heap, left);
	}
	if(right<heap->size && heap ->array[right]->count < heap -> array[index] -> count)
	{
		//swap the right to the index
		temp = heap -> array[index];
		heap -> array[index] = heap -> array[right];
		heap -> array[right] = temp;
		heapify(heap, right);
	}
	return;
}

node* getMinNodeHeap(MinHeap *heap)
{
	node *toReturn = heap -> array[0];
	heap -> array[0]= heap -> array[heap -> size-1];
	heap->size --;
	heapify(heap,0);
	return toReturn;
}

void insertNode(MinHeap *heap,node *toInsert)
{
	int curr = heap->size;
	while(curr!=0 && toInsert->count < heap->array[(curr-1)/2]->count)
	{
		heap->array[curr]= heap->array[(curr-1)/2];
		curr=(curr-1)/2;
	}
	heap->array[curr]=toInsert;
	heap->size++;
	return;
}

MinHeap* initMinHeap(char **contents, int* counts, int many)
{
	MinHeap *toReturn = (MinHeap*) malloc (sizeof(MinHeap));
	toReturn -> size = many;
	toReturn -> array = createNodeArray(contents, counts, many);
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
node** createNodeArray(char** contents,int* counts,int many)
{
	node** array = (node**)malloc(sizeof(node*)*many);
	int i=0;
	node* temp;
	for(i=0;i<many;i++)
	{
		temp=(node*)malloc(sizeof(node));
		temp->count=counts[i];
		temp->data=contents[i];
		temp->left=temp->right=NULL;
		array[i]=temp;
	}
	return array;
}

/* Create Huffman Tree Structure
 * Required Inputs:
 * Char *contents
 * Int* counts
 * int many how many entries
 * Sequence: As before
 * Outputs:
 * Head node pointer to the huffman tree
 */

node* createHuffmanTree(char** contents,int* counts,int many)
{
	MinHeap heap = initMinHeap(contents,counts,many);
	node *temp1,*temp2,*temp3;
	while(heap->size!=1)
	{
		//get two least element from both heap
		temp1=getMinNodeHeap(heap);
		temp2=getMinNodeHeap(heap);
		temp3=(node*) malloc (sizeof(node));
		temp3->left = temp1;
		temp3->right = temp2;
		temp3->count = temp1->count+temp2->count;
		insertNode(heap, temp3);
	}
	return heap->array[0];
}

void createCodeBook(int* code)
