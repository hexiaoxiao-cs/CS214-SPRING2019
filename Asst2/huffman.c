#include<stdio.h>
#include<stdlib.h>
typedef struct node{
	int count;
	struct node *left,*right;
	char* data;
}node;

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
	node** arr1,arr2;
	int arr1h=0,arr2h=0;
	arr1 = createNodeArray(contents, counts, many);
	arr2 = (node**)malloc(sizeof(node*)*many);
	arr1h = many-1;
	while(arr1h!=0)
	{
		//get two least element from both heap
		
	}
	return NULL;
}
