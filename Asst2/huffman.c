#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      //File system
#include <dirent.h>     //Directories
#include <unistd.h>
#include <libgen.h>

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

void appendSequenceExpandable(expandable* space, const char* sequence, int sequence_size) {
	//TODO: performance can be improved in here
	for(int i=0;i < sequence_size;i++) {
		appendExpandable(space, sequence[i]);
	}
}

void zeroUnusedExpandable(expandable* space) {
	memset(space->data + space->size, 0, space->total_size - space->size);
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
void buildHuffmanTreeFromFrequencies(expandable**, int*, int);
void buildHuffmanTreeFromRaw(char**, expandable**, int);
void DongFeng41KuaiDi(node*);
void LaunchDongFengDaoDan();
void TraverseTreePrefix(char**, expandable **, char*, int *, int*, node*);
void fillCodeBookFromTree(char**, expandable **);
void buildHuffmanTreeFromCodebook(const char* codebook_path);
void dumpCodeBookToFileRaw(const char* codebook_path, const char** codes, expandable** words, int size);
void doShits(const char* dir, int has_codebook, const char* codebook_path);
void undoShitDbg(const char* file);
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

void buildHuffmanTreeFromFrequencies(expandable** contents, int* counts, int many)
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

void buildHuffmanTreeFromRaw(char** codes, expandable** words, int many) {
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
	size=many;
	return;
}


//DongFeng-41KuaiDi is an atomic bomb for bombing the global memory -_-
void DongFeng41KuaiDi(node* curr)
{
	if (curr == NULL) { return; }
	DongFeng41KuaiDi(curr->left);
	DongFeng41KuaiDi(curr->right);
	destroyExpandable(curr->data);
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
		//printf("%s\n", currnode->data->data);
		curr[*nowcode] = '\0';
		stuff = (char*)malloc((size+1) * sizeof(char));
		memcpy(stuff,curr,sizeof(char)*(size+1));
		codes[*nowword] = stuff;
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
void fillCodeBookFromTree(char** codes, expandable **words) {
	//DEBUG
	//	codes[0] = "0";
	//	codes[1] = "100";
	//	codes[2] = "101";
	//	codes[3] = "1100";
	//	codes[4] = "1101";
	//	codes[5] = "111";
	//
	//	words[0] = createExpandable();
	//	appendSequenceExpandable(words[0], "and", 3);
	//	words[1] = createExpandable();
	//	appendSequenceExpandable(words[1], "cat", 3);
	//	words[2] = createExpandable();
	//	appendSequenceExpandable(words[2], "button", 6);
	//	words[3] = createExpandable();
	//	appendSequenceExpandable(words[3], "a", 1);
	//	words[4] = createExpandable();
	//	appendSequenceExpandable(words[4], "dog", 3);
	//	words[5] = createExpandable();
	//	appendSequenceExpandable(words[5], "\n", 1);
	//
	//	return;
	char* curr = (char*)malloc(sizeof(char)*(size+1));
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

int main()
{
	buildHuffmanTreeFromCodebook("./data/out.book");
	undoShitDbg("./data/huffman2.hcz");
	//doShit("./data/");
	return 0;
	//    createHuffmanForDecompress("test.codebook");
	//    writeHuffmanCodeBook("test.codebook2", 6);
	//    return 0;
	//    char *a[6];
	//    a[0] = "a";
	//    a[1] = "\n";
	//    a[2] = "cat";
	//    a[3] = "button";
	//    a[4] = "tall";
	//    a[5] = "and";
	//    int i = 0;
	//    char **b, **e;
	//    int c[6] = { 5,9,12, 13 ,16,45 };
	//    createHuffmanFromFrequency(a, c, 6);
	//    size = 6;
	//    b = (char**)malloc(sizeof(char*)*size);
	//    e = (char**)malloc(sizeof(char*)*size);
	//    createCodeBook(b, e);
	//    if (tree == NULL) { printf("NULL TREE"); return; }
	//    for (i = 0; i<6; i++)
	//    {
	//        printf("%s\t%s\n", b[i], e[i]);
	//    }
	//
	//
	//    return 0;
}


//This function allocate size + 1 bytes for data
//Content is guaranteed zero-terminated, however there might be zero in
//the middle of the content
void readFile(const char* file_path, char** data, int* size) {
	int handler = open(file_path, O_RDONLY);
	//TODO: check open error status
	//Blocking and readAll
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
	close(handler);	//Close file
}

void writeFile(const char* file_path, char* data, int size) {
	int handler = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
	//TODO: check open error status
	//Blocking and writeAll
	int tmp, ret;
	tmp = 0;
	while (tmp < size) {
		ret = write(handler, data + tmp, size - tmp);
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
	close(handler);	//Close file
}

int isDelim(char c) {
	return !isalnum(c) && !ispunct(c);
}

/* HuffmanCodeBook Format
* <valid token count>
* <ASCII bytestring> <\t> <token> <\n>
* ...
* ...
* terminated with new line(\n)
*
*/

void dumpCodeBookToFileRaw(const char* codebook_path, const char** codes, expandable** words, int size) {
	int i;
	char hex[3];	//hex buffer
	hex[2] = 0;

	expandable* content = createExpandable();

	content->size += sprintf(content->data, "%d\n", size);	//WARNING: this relies on the assumption that 49 bytes can hold the size

	for(i = 0;i < size;i++) {
		appendSequenceExpandable(content, codes[i], strlen(codes[i]));
		appendExpandable(content, '\t');
		if(words[i]->size == 1) {
			//Need to check possible control character
			if(isDelim(words[i]->data[0])) {
				//blank or control codes
				appendExpandable(content, '0');
				//encode before append
				sprintf(hex, "%02X", (unsigned char)(words[i]->data[0]));
				appendSequenceExpandable(content, hex, 2);
			} else
			goto normal;
		} else {
			normal:
			//normal sequence
			appendExpandable(content, '1');
			appendSequenceExpandable(content, words[i]->data, words[i]->size);
		}
		appendExpandable(content, '\n');
	}

	//free stuffs
	writeFile(codebook_path, content->data, content->size);
	destroyExpandable(content);
}

int loadCodeBookFromFile(const char* codebook_path, char*** codes_out, expandable*** words_out) {
	char* codebook_data;
	int codebook_size, i, counter = 0, lines, control_code;
	expandable* space;

	readFile(codebook_path, &codebook_data, &codebook_size);

	sscanf(codebook_data, "%d\n%n", &lines, &counter);

	*codes_out = malloc(lines * sizeof(char*));
	*words_out = malloc(lines * sizeof(expandable*));

	char** codes = *codes_out;
	expandable** words = *words_out;

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
	return lines;
}

void buildHuffmanTreeFromCodebook(const char* codebook_path) {
	int i;
	char** codes;
	expandable** words;
	int lines = loadCodeBookFromFile(codebook_path, &codes, &words);
	buildHuffmanTreeFromRaw(codes, words, lines);
	for(i = 0;i < lines;i++) {
		free(codes[i]);
	}
	free(codes);
	free(words);	//cannot free the individual expandable as it need to remain valid in the tree, but we can free the array
}

typedef struct {
	expandable* token;
	int freq;
} __attribute__((packed)) counter_t;

typedef struct {
	counter_t* counters;
	int in_use;
	int counters_size;
}counters_t;

void incrementTokenFrequency(const char* token, int token_size, counters_t* counters) {
	int i=0;
	counter_t* tmp;
	for(i=0;i < counters->counters_size;i++) {
		if(counters->counters[i].token == NULL)
		break;
		if(token_size == counters->counters[i].token->size && memcmp(counters->counters[i].token->data, token, token_size) == 0) {
			counters->counters[i].freq++;
			return;
		}
	}
	add:
	//cannot find an existed token inside counters
	for(i = 0;i < counters->counters_size;i++) {
		if(counters->counters[i].token == NULL) {
			counters->counters[i].token = createExpandable();
			appendSequenceExpandable(counters->counters[i].token, token, token_size);
			counters->counters[i].freq = 1;
			counters->in_use++;
			return;
		}
	}

	//unable to find empty entries
	tmp = realloc(counters->counters, (counters->counters_size + 100) * sizeof(counter_t));	//reallocate memory
	if(tmp == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}
	counters->counters = tmp;
	memset((char*)(counters->counters) + counters->counters_size * sizeof(counter_t), 0, 100 * sizeof(counter_t));
	counters->counters_size += 100;
	goto add;
}

/*
* 1: token loaded
* 0: no more token available
*/
int nextToken(expandable* buffer, const char* file_data, int file_size, int* idx) {
	if(*idx == file_size)
	return 0;
	if(isDelim(file_data[*idx])) {
		appendExpandable(buffer, file_data[*idx]);
		(*idx)++;
		return 1;
	}
	for(;*idx < file_size;(*idx)++) {
		if(!isDelim(file_data[*idx])) {
			//not a delimeter
			appendExpandable(buffer, file_data[*idx]);
		} else {
			//find a token
			return 2;
		}
	}
	return 3; //this should be the last token
}

void counting(const char* file_data, int file_size, counters_t* counters) {
	expandable* space = createExpandable();
	int offset = 0;
	while(nextToken(space, file_data, file_size, &offset) > 0) {
		incrementTokenFrequency(space->data, space->size, counters);  //this copies the token
		space->size = 0;  //reset expandable
	}
	destroyExpandable(space);
}

int qsort_cmp(const void* a, const void* b) {
	int freq_a = ((counter_t*)a)->freq;
	int freq_b = ((counter_t*)b)->freq;
	if(freq_a < freq_b)
	return -1;
	else if(freq_a > freq_b)
	return 1;
	else
	return 0;
}

void compress(expandable* buffer, char* file_data, int file_size, char** codes, expandable** words, int book_size) {
	expandable* tmp = createExpandable();
	int idx = 0, i, dbg;
	while((dbg = nextToken(tmp, file_data, file_size, &idx)) > 0) {
		for(i = 0;i < book_size;i++) {
			if(words[i]->size == tmp->size && memcmp(words[i]->data, tmp->data, tmp->size) == 0) {
				//found a match in the book
				appendSequenceExpandable(buffer, codes[i], strlen(codes[i]));
				goto nextone;
			}
		}
		printf("Possible memory corruption detected, unable to find match in codebook!\n");
		exit(1);
		nextone:
		tmp->size = 0;
	}
	destroyExpandable(tmp);
}

void decompress(expandable* buffer, char* file_data, int file_size, node* tree) {
	expandable* tmp = createExpandable();
	node* star = tree;
	for(int i=0;i < file_size;i++) {
		if(file_data[i] == '0') {
			star = star->left;
		} else if(file_data[i] == '1') {
			star = star->right;
		} else {
			printf("Unexpected content encountered.\n");
			exit(1);
		}
		if(star->left == NULL && star->right == NULL) {
			//leaf node
			appendSequenceExpandable(buffer, star->data->data, star->data->size);
			star = tree;
		}
	}
	destroyExpandable(tmp);
}

void undoShitDbg(const char* file) {
	expandable* buffer = createExpandable();
	char* file_data;
	int file_size;
	readFile(file, &file_data, &file_size);
	decompress(buffer, file_data, file_size, tree);
	writeFile("out.txt", buffer->data, buffer->size);
	destroyExpandable(buffer);
	free(file_data);
}

void buildHuffmanTreeFromCounters(counters_t* counters) {
	//sort counters
	int i;
	qsort(counters->counters, counters->in_use, sizeof(counter_t), qsort_cmp);
	int* frequencies = malloc(sizeof(int) * counters->counters_size);
	expandable** tokens = malloc(sizeof(expandable*) * counters->counters_size);
	for(i = 0;i < counters->in_use;i++) {
		frequencies[i] = counters->counters[i].freq;
		tokens[i] = counters->counters[i].token;
	}
	buildHuffmanTreeFromFrequencies(tokens, frequencies, counters->in_use);
	free(tokens);
	free(frequencies);
}

void dumpCodeBookToPathRaw(const char* dir, char** codes, expandable** words, int items_count) {
	expandable* codebook_path = createExpandable();
	appendSequenceExpandable(codebook_path, dir, strlen(dir));
	appendExpandable(codebook_path, '/');
	appendSequenceExpandable(codebook_path, "HuffmanCodebook", 15);
	dumpCodeBookToFileRaw(codebook_path->data, codes, words, items_count);
	destroyExpandable(codebook_path);
}

void loadCodeBookFromTree(char*** codes, expandable*** words, int items_count) {
	*codes = malloc(sizeof(char*) * items_count);
	*words = malloc(sizeof(expandable*) * items_count);
	fillCodeBookFromTree(*codes, *words);
}

void compressFile(const char* original_file, expandable* path_buffer, expandable* buffer, char* file_data, int file_size, char** codes, expandable** words, int items_count) {
	compress(buffer, file_data, file_size, codes, words, items_count);
	appendSequenceExpandable(path_buffer, original_file, strlen(original_file));
	appendSequenceExpandable(path_buffer, ".hcz", 4);
	zeroUnusedExpandable(path_buffer);
	writeFile(path_buffer->data, path_buffer->data, path_buffer->size);
}

void cleanHalfCodeBooks(char** codes, expandable** words, int items_count) {
	int i;
	for(i = 0; i < items_count;i++) {
		free(codes[i]);
	}
	free(codes);
	free(words);
}

void doSingleShit(const char* filepath, int has_codebook, const char* codebook_path) {
	char* file_data;
	char** codes;
	expandable** words;
	int file_size, i;
	int items_count;

	readFile(filepath, &file_data, &file_size);
	if(!has_codebook) {
		counters_t counters;
		counters.counters = calloc(10, sizeof(counter_t));
		counters.counters_size = 10;
		counters.in_use = 0;

		counting(file_data, file_size, &counters);
		buildHuffmanTreeFromCounters(&counters);
		loadCodeBookFromTree(&codes, &words, counters.in_use);
		items_count = counters.in_use;

		//dump codebook if we dont have one yet
		char* filepath_dup = strdup(filepath);
		char* dir = dirname(filepath_dup);
		dumpCodeBookToPathRaw(dir, codes, words, items_count);
		free(filepath_dup);
	} else {
		//use existed codebook
		items_count = loadCodeBookFromFile(codebook_path, &codes, &words);
	}

	expandable* output_buffer = createExpandable();
	expandable* output_path = createExpandable();

	compressFile(filepath, output_path, output_buffer, file_data, file_size, codes, words, items_count);
	free(file_data);

	cleanHalfCodeBooks(codes, words, items_count);
	destroyExpandable(output_path);
	destroyExpandable(output_buffer);
}

void doShits(const char* dir, int has_codebook, const char* codebook_path) {
	char* command, *task_data, *line, *file_data;
	int task_size, file_size, i, items_count;

	char** codes;
	expandable** words;


	asprintf(&command, "find %s -type f > output.tmp", dir);
	system(command);
	free(command);
	readFile("output.tmp", &task_data, &task_size);

	if(!has_codebook) {
		//need to count and build tree first
		counters_t counters;
		counters.counters = calloc(10, sizeof(counter_t));
		counters.counters_size = 10;
		counters.in_use = 0;

		char* task_data_dup = strdup(task_data);
		line = strtok(task_data_dup, "\n");
		while(line) {
			printf("B: %s\n", line);
			readFile(line, &file_data, &file_size);
			counting(file_data, file_size, &counters);
			free(file_data);
			line = strtok(NULL, "\n");
		}

		free(task_data_dup);

		//create huffman tree
		//sort counters
		buildHuffmanTreeFromCounters(&counters);
		loadCodeBookFromTree(&codes, &words, counters.in_use);
		items_count = counters.in_use;

		dumpCodeBookToPathRaw(codebook_path, codes, words, items_count);

	} else {
		//use existed codebook
		loadCodeBookFromFile(codebook_path, &codes, &words);
		items_count = size;
	}

	line = strtok(task_data, "\n");

	expandable* output_buffer = createExpandable();
	expandable* output_path = createExpandable();
	while(line) {
		printf("C: %s.hcz\n", line);
		readFile(line, &file_data, &file_size);
		compressFile(line, output_path, output_buffer, file_data, file_size, codes, words, items_count);
		free(file_data);
		line = strtok(NULL, "\n");
	}

	cleanHalfCodeBooks(codes, words, items_count);

	free(task_data);
	destroyExpandable(output_path);
	destroyExpandable(output_buffer);
}
