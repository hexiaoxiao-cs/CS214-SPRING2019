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
	free(heap->array);
	free(heap);
	return;
}

/*
	createHuffmanFromCodeBook
	Required
	Codes ending with \0
	words ending with \0
*/

void createHuffmanFromCodeBook(char** codes, const char** words, int many) {
	//Initialize huffman tree from codebook
	int temp=0,cnt=0;
	node* root = (node*)malloc(sizeof(node));
	root->left=root->right=NULL;
	node* curr=root;
	for(temp=0;temp<many;temp++)
	{
		cnt=0;
		curr=root;
		while(code[temp][cnt]!='\0')
		{
			if(code[temp][cnt]=='0'){
				if(curr->left==NULL){
					curr->left=(node*)malloc(sizeof(node));
					curr->left->left=curr->left->right=NULL;
				}
			curr=curr->left;
			}
			if(code[temp][cnt]=='1'){
				if(curr->right==NULL){
					curr->right=(node*)malloc(sizeof(node));
					curr->right->left=curr->right->right=NULL;
				}
			curr=curr->right;
			}
			cnt++;
		}
		curr->data=word[temp];
	}
	tree=root;
	return;
}


//DongFeng-41KuaiDi is an atomic bomb for bombing the global memory -_-
void DongFeng41KuaiDi (node* curr)
{
	if(curr==NULL){return;}
	DongFeng41KuaiDi(curr->left);
	DongFeng41KuaiDi(curr->right);
	free(curr);
	return;
}

void LaunchDongFengDaoDan()
{
	DongFeng41KuaiDi(tree);
	size=0;
}
void TraverseTreePrefix(char** codes, char **words, char* curr,int *nowcode, int* nowword, node* currnode)
{
	//At the edge which means that must be a value node
	if(currnode->left==currnode->right==NULL){
		words[nowword]=currnode->data;
		codes[nowword]=curr;
		curr[nowcode]= '\0';
		(&nowcode)--;
		(&nowword)++;
		return;
	}
	//Left Node
	{
		//Begin Accessing
		(&nowcode)++;
		curr[nowcode]='0';
		TraverseTreePrefix(codes,words,curr,nowcode,nowword,currnode->left)
		//Finished Accessing--Cleaning
		(&nowcode)--;
		curr[nowcode]='\0';
	}
	//Right Node
	{
		//Begin Accessing
		(&nowcode)++;
		curr[nowcode]='1';
		TraverseTreePrefix(codes,words,curr,nowcode,nowword,currnode->right)
		//Finished Accessing--Cleaning
		(&nowcode)--;
		curr[nowcode]='\0';
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
void createCodeBook(char** codes, char **words) {
	char* curr = (char*) malloc (sizeof(char)*size);
	int nowcode=0,nowword=0;
	if(tree==NULL) return; //For security, check whether there is a Huffman Tree
	codes = (char**) malloc (sizeof(char*)*size);
	words = (char**) malloc (sizeof(char*)*size);
	TraverseTreePrefix(codes,words,curr, &nowcode,&nowword,tree);
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

//This function allocate size + 1 bytes for data
//Content is guaranteed zero-terminated, however there might be zero in
//the middle of the content
void readFile(const char* file_path, char** data, int* size) {
	int handler = open(file_path, O_RDONLY);
	//TODO: check open error status
	//Nonblock and readAll
	int tmp, ret;
	int file_size = lseek(handler, 0, SEEK_END);
	char* huge_shit = (char*)malloc(file_size + 1);
	huge_shit[file_size] = 0;	//Zero terminated
	tmp = 0;
	while(tmp < file_size) {
		ret = read(handler, huge_shit + tmp, file_size - tmp);
		if(ret < 0) {
			//TODO error checking
		} else if(ret == 0) {
			break;
		} else {
			//Positive interger
			tmp += ret;
		}
	}
	*data = huge_shit;
	*size = file_size;
}

void createHuffmanForDecompress(const char* codebook_path) {
	char* codebook_data;
	int codebook_size;
	readFile(codebook_path, &codebook_data, &codebook_size);

}
