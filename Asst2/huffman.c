#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      //File system
#include <dirent.h>     //Directories

//Dynamic array
typedef struct {
		char* data;
		int size;
		int total_size;
} expandable;

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

void expandExpandable(expandable* space) {
		space->total_size = space->total_size + 50;
		space->data = realloc(space->data, space->total_size + 1);  //null terminator (reserved for codes)
		space->data[space->total_size] = 0; //set it to null
}

void appendExpandable(expandable* space, char c) {
		space->data[space->size++] = c;
		if(space->size == space->total_size) {
				expandExpandable(space);
		}
}

typedef struct node {
	int count;
	struct node *left, *right;
	expandable* data;
}node;
typedef struct MinHeap {
	int size;
	struct node** array;
}MinHeap;


//HUFFMAN TREE
node* tree;
int size = 0;
//start to declear functions
void heapify(MinHeap*, int);
node* getMinNodeHeap(MinHeap*);
void insertNode(MinHeap*, node*);
MinHeap* initMinHeap(expandable**, int*, int);
node** createNodeArray(expandable**, int*, int);
void createHuffmanFromFrequency(expandable**, int*, int);
void createHuffmanFromCodeBook(char**, expandable**, int);
void DongFeng41KuaiDi(node*);
void LaunchDongFengDaoDan();
void TraverseTreePrefix(char**, expandable **, char*, int *, int*, node*);
void createCodeBook(char**, expandable **);
void createHuffmanForDecompress(const char* codebook_path);
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

MinHeap* initMinHeap(expandable **contents, int* counts, int many)
{
	MinHeap *toReturn = (MinHeap*)malloc(sizeof(MinHeap));
	toReturn->size = many;
	toReturn->array = createNodeArray(contents, counts, many);
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
node** createNodeArray(expandable** contents, int* counts, int many)
{
	node** array = (node**)malloc(sizeof(node*)*many);
	int i = 0;
	node* temp;
	size = many;
	for (i = 0; i<many; i++)
	{
		temp = (node*)malloc(sizeof(node));
		temp->count = counts[i];
		temp->data = contents[i];
		temp->left = temp->right = NULL;
		array[i] = temp;
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

void createHuffmanFromFrequency(expandable** contents, int* counts, int many)
{
	MinHeap* heap = initMinHeap(contents, counts, many);
	node *temp1, *temp2, *temp3;
	while (heap->size != 1)
	{
		//get two least element from both heap

		temp1 = getMinNodeHeap(heap);
		temp2 = getMinNodeHeap(heap);
		//printf("Merging %s and %s", temp1->data, temp2->data);
		temp3 = (node*)malloc(sizeof(node));
		temp3->left = temp1;
		temp3->right = temp2;
		temp3->count = temp1->count + temp2->count;
		insertNode(heap, temp3);
	}
	tree = heap->array[0];
	free(heap->array);
	free(heap);
	return;
}

/*
createHuffmanFromCodeBook
Required
Codes ending with \out0
words ending with \0
*/

void createHuffmanFromCodeBook(char** codes, expandable** words, int many) {
	//Initialize huffman tree from codebook
	int temp = 0, cnt = 0;
	node* root = (node*)malloc(sizeof(node));
	root->left = root->right = NULL;
	node* curr = root;
	for (temp = 0; temp<many; temp++)
	{
		cnt = 0;
		curr = root;
		while (codes[temp][cnt] != '\0')
		{
			if (codes[temp][cnt] == '0') {
				if (curr->left == NULL) {
					curr->left = (node*)malloc(sizeof(node));
					curr->left->left = curr->left->right = NULL;
				}
				curr = curr->left;
			}
			if (codes[temp][cnt] == '1') {
				if (curr->right == NULL) {
					curr->right = (node*)malloc(sizeof(node));
					curr->right->left = curr->right->right = NULL;
				}
				curr = curr->right;
			}
			cnt++;
		}
		curr->data = words[temp];
	}
	tree = root;
	return;
}


//DongFeng-41KuaiDi is an atomic bomb for bombing the global memory -_-
void DongFeng41KuaiDi(node* curr)
{
	if (curr == NULL) { return; }
	DongFeng41KuaiDi(curr->left);
	DongFeng41KuaiDi(curr->right);
	free(curr);
	return;
}

void LaunchDongFengDaoDan()
{
	DongFeng41KuaiDi(tree);
	size = 0;
}
void TraverseTreePrefix(char** codes, expandable **words, char* curr, int *nowcode, int* nowword, node* currnode)
{
	char *stuff;
	//At the edge which means that must be a value node
	if (currnode->left == NULL &&  currnode->right == NULL) {
		words[*nowword] = currnode->data;
		printf("%s\n", currnode->data->data);
		curr[*nowcode] = '\0';
		stuff = (char*)malloc(size * sizeof(char));
		memcpy(stuff,curr,sizeof(char)*size);
		codes[*nowword] = stuff;
		printf("%s\n", codes[*nowword]);
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
void createCodeBook(char** codes, expandable **words) {
	char* curr = (char*)malloc(sizeof(char)*size);
	int nowcode = 0, nowword = 0;
	if (tree == NULL) return; //For security, check whether there is a Huffman Tree
	TraverseTreePrefix(codes, words, curr, &nowcode, &nowword, tree);
}

//FILE SYSTEM RELATED STUFF
//TODO:
//1. read from codebook file
//2. read file to count frequencies
//3. recursively traverse directories
//4. compress according to the code code book
//5. decompress according to constructed tree

//Test Driver For Xiaoxiao He's Huffman Tree

void main()
{
	createHuffmanForDecompress("test.codebook");

	return;
	char *a[6];
	a[0] = "a";
	a[1] = "\n";
	a[2] = "cat";
	a[3] = "button";
	a[4] = "tall";
	a[5] = "and";
	int i = 0;
	char **b, **e;
	int c[6] = { 5,9,12, 13 ,16,45 };
	createHuffmanFromFrequency(a, c, 6);
	size = 6;
	b = (char**)malloc(sizeof(char*)*size);
	e = (char**)malloc(sizeof(char*)*size);
	createCodeBook(b, e);
	if (tree == NULL) { printf("NULL TREE"); return; }
	for (i = 0; i<6; i++)
	{
		printf("%s\t%s\n", b[i], e[i]);
	}


	return;
}


//This function allocate size + 1 bytes for data
//Content is guaranteed zero-terminated, however there might be zero in
//the middle of the content
void readFile(const char* file_path, char** data, int* size) {
	int handler = open(file_path, O_RDONLY);
	//TODO: check open error status
	//Nonblock and readAll
	int tmp, ret;
	int file_size = lseek(handler, 0, SEEK_END);
	lseek(handler, 0, SEEK_SET);
	char* huge_shit = (char*)malloc(file_size + 1);
	huge_shit[file_size] = 0;       //Zero terminated
	tmp = 0;
	while (tmp < file_size) {
		ret = read(handler, huge_shit + tmp, file_size - tmp);
		if (ret < 0) {
			//TODO error checking
		}
		else if (ret == 0) {
			break;
		}
		else {
			//Positive interger
			tmp += ret;
		}
	}
	*data = huge_shit;
	*size = file_size;
}

/* HuffmanCodeBook Format
 * <valid token count>
 * <ASCII bytestring> <\t> <token> <\n>
 * ...
 * ...
 * terminated with new line(\n)
 *
 */

void createHuffmanForDecompress(const char* codebook_path) {
	char* codebook_data;
	int codebook_size, i, counter = 0, lines, control_code;
	expandable* space;

	readFile(codebook_path, &codebook_data, &codebook_size);

	sscanf(codebook_data, "%d\n%n", &lines, &counter);

	char** codes = malloc(lines * sizeof(char*));
	expandable** words = malloc(lines * sizeof(expandable*));

	for(i = 0;i < lines;i++) {
		space = createExpandable();

		//Read strings until we hit a tab
		while(codebook_data[counter] != '\t') {
			appendExpandable(space, codebook_data[counter++]);
		}
		//byte string created
		codes[i] = space->data;
		destroyExpandableWithoutFree(space);

		//this is a tab
		counter++;

		space = createExpandable();
		//first byte of token determine the type of token
		if(codebook_data[counter++] == '1') {
			//normal token
			while(codebook_data[counter] != '\n') {
				appendExpandable(space, codebook_data[counter++]);
			}
			words[i] = space;
		} else {
			//control code token
			sscanf(codebook_data + counter, "%02X", &control_code);
			space->data[0] = (char)control_code;
			space->size = 1;
			counter += 2; //2 bytes for codes
			words[i] = space;
		}
		counter += 1; //consume newline
	}
	free(codebook_data);
	createHuffmanFromCodeBook(codes, words, lines);
}
