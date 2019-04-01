#include <stddef.h>
//Dynamic array
typedef struct {
	char* data;
	size_t size;
	size_t total_size;
} expandable;

typedef struct {
	void** data;
	size_t size;
	size_t total_size;
} expandablePtr;


typedef struct node {
	int count;
	struct node *left, *right;
	expandable* data;
	expandable* codes;
} node;

typedef struct MinHeap {
	int size;
	struct node** array;
} MinHeap;

//HUFFMAN TREE
extern node* tree;
extern int size;


void heapify(MinHeap*, int);
node* getMinNodeHeap(MinHeap*);
void insertNode(MinHeap*, node*);
MinHeap* initMinHeap(node** nodearray, int many);
node** createNodeArray(expandable**, int*, int);
void TraverseTreePrefix(expandable** codes, expandable **words, char* curr, int *nowcode, int* nowword, node* currnode);

expandable* createExpandable();
void destroyExpandable(expandable* space);
void destroyExpandableWithoutFree(expandable* space);
void expandExpandable(expandable* space, size_t size);
void appendExpandable(expandable* space, char c);
void appendSequenceExpandable(expandable* space, const char* sequence, size_t sequence_size);
void zeroUnusedExpandable(expandable* space);

expandablePtr* createExpandablePtr();
void destroyExpandablePtr(expandablePtr* space);
void destroyExpandablePtrWithoutFree(expandablePtr* space);
void expandExpandablePtr(expandablePtr* space);
void appendExpandablePtr(expandablePtr* space, const void* c);
void DongFeng41KuaiDi(node*);
void LaunchDongFengDaoDan();
