//Dynamic array
typedef struct {
	char* data;
	int size;
	int total_size;
} expandable;

typedef struct {
	void** data;
	int size;
	int total_size;
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
void expandExpandable(expandable* space, int size);
void appendExpandable(expandable* space, char c);
void appendSequenceExpandable(expandable* space, const char* sequence, int sequence_size);
void zeroUnusedExpandable(expandable* space);

expandablePtr* createExpandablePtr();
void destroyExpandablePtr(expandablePtr* space);
void destroyExpandablePtrWithoutFree(expandablePtr* space);
void expandExpandablePtr(expandablePtr* space);
void appendExpandablePtr(expandablePtr* space, const void* c);
void appendSequenceExpandablePtr(expandablePtr* space, void* const * sequence, int sequence_size);
void zeroUnusedExpandablePtr(expandablePtr* space);

void DongFeng41KuaiDi(node*);
void LaunchDongFengDaoDan();
