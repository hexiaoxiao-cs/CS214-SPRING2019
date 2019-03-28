#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>	//File system
#include <dirent.h>	//Directories


typedef struct node{
	int count;
	struct node *left,*right;
	char* data;
}node;
typedef struct MinHeap {
    int size;
    struct node** array;
}Minheap;
//HUFFMAN TREE
node* tree;
int size=0;

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
	size=many;
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

void createHuffmanFromFrequency(char** contents,int* counts,int many)
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
	tree=heap->array[0];
	free(heap);
	return;
}

void createHuffmanFromCodeBook(char** codes, const char** words, int many) {
	//Initialize huffman tree from codebook

}
/* Create Huffman Codebook From Huffman Tree
	 Required:
	 Executed createHuffmanFromFrequency
	 tree Not NULL
	 Return:
	 Codes: Encoded Huffman Codes
	 Words: The corrosponding Words
	 size: The size of the Array
	 Rule:
	 prefix Traverse
*/

//DongFeng-41 is an atomic bomb for bombing the global memory -_-
void DongFeng-41 ()
{
	free(tree);
	size=0;
}
void TraverseTreePrefix(char** codes, char **words, char* curr,int *nowcode, int* nowword, node* currnode)
{
	if(currnode->left==currnode->right==NULL){
		words[nowword]=currnode->data;
		codes[nowword]=curr;
		curr[nowcode]= 

	}
}

void createCodeBook(char** codes, char **words) {
	if(tree==NULL) return; //For security, check whether there is a Huffman Tree
	codes = (char**) malloc (sizeof(char*)*size);
	words = (char**) malloc (sizeof(char*)*size);

}

//FILE SYSTEM RELATED STUFF
//TODO:
//1. read from codebook file
//2. read file to count frequencies
//3. recursively traverse directories
//4. compress according to the code code book
//5. decompress according to constructed tree

/* HuffmanCodeBook Format
 *
 * <ASCII bytestring> <\t> <token> <\n>
 * ...
 * ...
 * terminated with new line(\n)
 *
 */

void createHuffmanForDecompress(const char* codebook_path) {
	int handler = open(codebook_path, O_RDONLY);



	close(handler);
}
