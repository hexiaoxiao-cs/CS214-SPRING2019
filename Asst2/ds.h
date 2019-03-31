//Dynamic array
typedef struct {
	char* data;
	int size;
	int total_size;
} expandable;

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
MinHeap* initMinHeap(expandable**, int*, int);
node** createNodeArray(expandable**, int*, int);
void TraverseTreePrefix(char**, expandable **, char*, int *, int*, node*);

expandable* createExpandable();
void destroyExpandable(expandable* space);
void destroyExpandableWithoutFree(expandable* space);
void expandExpandable(expandable* space);
void appendExpandable(expandable* space, char c);
void appendSequenceExpandable(expandable* space, const char* sequence, int sequence_size);
void zeroUnusedExpandable(expandable* space);

void DongFeng41KuaiDi(node*);
void LaunchDongFengDaoDan();
