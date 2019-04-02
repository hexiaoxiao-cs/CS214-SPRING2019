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
#include <errno.h>

#include <search.h>

#include "huffman.h"

int size = 0;
node *tree = NULL;

void cleanHalfCodeBooks(expandable **codes, expandable **words, int items_count) {
    free(codes);
    free(words);
}

int bst_compare(const void *a, const void *b) {
    const node *a_node = (const node *) a;
    const node *b_node = (const node *) b;
    if (a_node->data->size == b_node->data->size) {
        return memcmp(a_node->data->data, b_node->data->data, a_node->data->size);
    } else {
        return a_node->data->size - b_node->data->size;
    }
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

void buildHuffmanTreeFromNodesArray(node **nodes, int many) {
    MinHeap *heap = initMinHeap(nodes, many); //Init Mean Heap
    node *temp1, *temp2, *temp3;
    while (heap->size != 1) {
        //get two least element from both heap

        temp1 = getMinNodeHeap(heap);
        temp2 = getMinNodeHeap(heap);
        //printf("Merging %s and %s", temp1->data, temp2->data);
        temp3 = (node *) malloc(sizeof(node));
        temp3->left = temp1;
        temp3->right = temp2;
        temp3->count = temp1->count + temp2->count;
        insertNode(heap, temp3);
    }
    tree = heap->array[0];
    size = many;
    free(heap->array);//reduce memory usage
    free(heap);//reduce memory usage
    return;
}

/*
createHuffmanFromCodeBook
Required
node** nodearray sorted in acsending sequence
int many how many nodes in the node array
Outputs:
None
*/

void buildHuffmanTreeFromCodes(node **nodearray, int many) {
    //Initialize huffman tree from codebook
    int temp = 0, cnt = 0;
    node *root = (node *) malloc(sizeof(node));
    root->left = root->right = NULL;
    node *curr = root;
    node *prev = root;
    for (temp = 0; temp < many; temp++) {
        cnt = 0;
        curr = root;
        prev = root;
        while (nodearray[temp]->codes->data[cnt] != '\0') {
            if (nodearray[temp]->codes->data[cnt] == '0') {
                if (curr->left == NULL) {
                    curr->left = (node *) malloc(sizeof(node));
                    curr->left->left = curr->left->right = NULL;
                }
                prev = curr;
                curr = curr->left;
            }
            if (nodearray[temp]->codes->data[cnt] == '1') {
                if (curr->right == NULL) {
                    curr->right = (node *) malloc(sizeof(node));
                    curr->right->left = curr->right->right = NULL;
                }
                prev = curr;
                curr = curr->right;
            }
            cnt++;
        }
        if (prev->left == curr) {
            free(curr);
            prev->left = nodearray[temp];
        }
        if (prev->right == curr) {
            free(curr);
            prev->right = nodearray[temp];
        }
    }
    tree = root;
    size = many;
    return;
}


//DongFeng-41KuaiDi is an atomic bomb for bombing the global memory -_-
void DongFeng41KuaiDi(node *curr) {
    if (curr == NULL) { return; }
    DongFeng41KuaiDi(curr->left);
    DongFeng41KuaiDi(curr->right);
    destroyExpandable(curr->data);
    free(curr);
    return;
}

void LaunchDongFengDaoDan() {
    DongFeng41KuaiDi(tree);
    size = 0;
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
void exportCodeFromHuffmanTree(expandable **codes, expandable **words) {
    char *curr = (char *) malloc(sizeof(char) * (size + 1));
    int nowcode = 0, nowword = 0;
    if (tree == NULL) return; //For security, check whether there is a Huffman Tree
    TraverseTreePrefix(codes, words, curr, &nowcode, &nowword, tree);
    free(curr);
}

//FILE SYSTEM RELATED STUFF

//Test Driver For Xiaoxiao He's Huffman Tree

void panic(const char *module, const char *reason, const char *extra) {
    if (extra) {
        printf("Error: %s (%s) [%s]\n", module, reason, extra);
    } else {
        printf("Error: %s (%s)\n", module, reason);
    }

    exit(1);
}

//This function allocate size + 1 bytes for data
//Content is guaranteed zero-terminated, however there might be zero in
//the middle of the content
void readFile(const char *file_path, char **data, size_t *size) {
    int handler = open(file_path, O_RDONLY);
    if (handler < 0) {
        //Unable to open specific files
        panic("Unable to open file", strerror(errno), file_path);
    }
    //Blocking and readAll
    size_t tmp, ret;
    size_t file_size = lseek(handler, 0, SEEK_END);
    lseek(handler, 0, SEEK_SET);
    char *huge_shit = (char *) malloc(file_size + 1);
    huge_shit[file_size] = 0;       //Zero terminated
    tmp = 0;
    while (tmp < file_size) {
        ret = read(handler, huge_shit + tmp, file_size - tmp);
        if (ret < 0) {
            panic("Error reading file", strerror(errno), file_path);
        } else if (ret == 0) {
            break;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    *data = huge_shit;
    *size = file_size;
    close(handler);    //Close file
}

void writeFile(const char *file_path, char *data, size_t size) {
    int handler = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (handler < 0) {
        //Unable to open specific files
        panic("Unable to open file", strerror(errno), file_path);
    }
    //Blocking and writeAll
    size_t tmp, ret;
    tmp = 0;
    while (tmp < size) {
        ret = write(handler, data + tmp, size - tmp);
        if (ret < 0) {
            panic("Error when writing file", strerror(errno), file_path);
        } else if (ret == 0) {
            break;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    close(handler);    //Close file
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

void dumpCodeBookToFileRaw(const char *codebook_path, expandable **const codes, expandable **words, int size) {
    int i;
    char hex[3];    //hex buffer
    hex[2] = 0;

    expandable *content = createExpandable();

    content->size += sprintf(content->data, "%d\n",
                             size);    //WARNING: this relies on the assumption that 49 bytes can hold the size

    for (i = 0; i < size; i++) {
        appendSequenceExpandable(content, codes[i]->data, codes[i]->size);
        appendExpandable(content, '\t');
        if (words[i]->size == 1) {
            //Need to check possible control character
            if (isDelim(words[i]->data[0])) {
                //blank or control codes
                appendExpandable(content, '0');
                //encode before append
                sprintf(hex, "%02X", (unsigned char) (words[i]->data[0]));
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

void addNodeToBST(void **tree, node *node) {
    tsearch(node, tree, bst_compare);
}

void loadBSTFromCodeBookFile(const char *codebook_path, void **BSTree) {
    char *codebook_data;
    size_t codebook_size, i;
    int control_code, lines, counter = 0;
    expandable *space;

    readFile(codebook_path, &codebook_data, &codebook_size);

    sscanf(codebook_data, "%d\n%n", &lines, &counter);

    for (i = 0; i < lines; i++) {
        node *new_node = calloc(1, sizeof(node));
        space = createExpandable();

        //Read strings until we hit a tab
        while (codebook_data[counter] != '\t') {
            appendExpandable(space, codebook_data[counter++]);
        }
        //byte string created
        new_node->codes = space;

        //this is a tab
        counter++;

        space = createExpandable();
        //first byte of token determine the type of token
        if (codebook_data[counter++] == '1') {
            //normal token
            while (codebook_data[counter] != '\n') {
                appendExpandable(space, codebook_data[counter++]);
            }
            new_node->data = space;
        } else {
            //control code token
            sscanf(codebook_data + counter, "%02X", &control_code);
            space->data[0] = (char) control_code;
            space->size = 1;
            counter += 2; //2 bytes for codes
            new_node->data = space;
        }
        counter += 1; //consume newline
        addNodeToBST(BSTree, new_node);
    }
    free(codebook_data);
}

//1->inserted	(should not free token)
//0->existed	(should free token);
int incrementTokenFrequency(node *token_node, void **BSTree) {
    node *returnedNode;
    if ((returnedNode = *((node **) tsearch(token_node, BSTree, bst_compare))) == token_node) {
        //newly inserted
        return 1;
    } else {
        //existed
        returnedNode->count++;
        return 0;
    }
}

/*
* 1: token loaded
* 0: no more token available
*/
int nextToken(expandable *buffer, const char *file_data, size_t file_size, size_t *idx) {
    if (*idx == file_size)
        return 0;
    if (isDelim(file_data[*idx])) {
        appendExpandable(buffer, file_data[*idx]);
        (*idx)++;
        return 1;
    }
    for (; *idx < file_size; (*idx)++) {
        if (!isDelim(file_data[*idx])) {
            //not a delimeter
            appendExpandable(buffer, file_data[*idx]);
        } else {
            //find a token
            return 2;
        }
    }
    return 3; //this should be the last token
}

void counting(const char *file_data, size_t file_size, void **BSTree) {
    expandable *space = createExpandable();
    node *newNode = calloc(1, sizeof(node));
    size_t offset = 0;

    newNode->count = 1;
    while (nextToken(space, file_data, file_size, &offset) > 0) {
        newNode->data = space;
        if (incrementTokenFrequency(newNode, BSTree) == 1) {
            space = createExpandable();    //original space ownership has been taken by BST
            newNode = calloc(1, sizeof(node));
            newNode->count = 1;
        } else {
            space->size = 0;  //reset expandable
        }
    }
    destroyExpandable(space);
    free(newNode);
}

int qsort_cmp(const void *a, const void *b) {
    int freq_a = (*((node **) a))->count;
    int freq_b = (*((node **) b))->count;
    if (freq_a < freq_b)
        return -1;
    else if (freq_a > freq_b)
        return 1;
    else
        return 0;
}

void compress(expandable *buffer, char *file_data, size_t file_size, void **BSTree) {
    expandable *tmp = createExpandable();
    size_t idx = 0, dbg;
    node dummy;
    node *found;

    while ((dbg = nextToken(tmp, file_data, file_size, &idx)) > 0) {
        dummy.data = tmp;
        found = *((node **) tfind(&dummy, BSTree, bst_compare));
        if (!found) {
            printf("Possible memory corruption detected, unable to find match in codebook!\n");
            exit(1);
        }
        appendSequenceExpandable(buffer, found->codes->data, found->codes->size);
        tmp->size = 0;    //reuse tmp
    }
    destroyExpandable(tmp);
}

void decompress(expandable *buffer, char *file_data, size_t file_size, node *huffman_tree) {
    node *star = tree;
    size_t i;
    for (i = 0; i < file_size; i++) {
        if (file_data[i] == '0') {
            star = star->left;
        } else if (file_data[i] == '1') {
            star = star->right;
        } else {
            printf("Unexpected content encountered.\n");
            exit(1);
        }
        if (star->left == NULL && star->right == NULL) {
            //leaf node
            appendSequenceExpandable(buffer, star->data->data, star->data->size);
            star = tree;
        }
    }
}

void dumpCodeBookToPathRaw(const char *dir, expandable **const codes, expandable **words, int items_count) {
    expandable *codebook_path = createExpandable();
    appendSequenceExpandable(codebook_path, dir, (int) strlen(dir));
    appendExpandable(codebook_path, '/');
    appendSequenceExpandable(codebook_path, "HuffmanCodebook", 15);
    dumpCodeBookToFileRaw(codebook_path->data, codes, words, items_count);
    destroyExpandable(codebook_path);
}

void loadCodeBookFromTree(expandable ***codes, expandable ***words, int items_count) {
    *codes = malloc(sizeof(expandable *) * items_count);
    *words = malloc(sizeof(expandable *) * items_count);
    exportCodeFromHuffmanTree(*codes, *words);
}

void useTmpFolderForFile(expandable *path_buffer, const char *original_file) {
    char *original_filename;
    char *original_file_dup = strdup(original_file);
    char *original_path = dirname(original_file_dup);
    appendSequenceExpandable(path_buffer, original_path, strlen(original_path));
    free(original_file_dup);

    appendSequenceExpandable(path_buffer, "/.tmp/", 6);

    original_file_dup = strdup(original_file);
    original_filename = basename(original_file_dup);
    appendSequenceExpandable(path_buffer, original_filename, strlen(original_filename));
    free(original_file_dup);
}

void compressFile(const char *original_file, expandable *path_buffer,
                  expandable *buffer, char *file_data, size_t file_size,
                  void **BSTree, int move_to_tmp_folder) {
    compress(buffer, file_data, file_size, BSTree);
    if (move_to_tmp_folder) {
        //need to use better file name
        useTmpFolderForFile(path_buffer, original_file);
    } else {
        appendSequenceExpandable(path_buffer, original_file, strlen(original_file));
    }
    appendSequenceExpandable(path_buffer, ".hcz", 4);
    zeroUnusedExpandable(path_buffer);
    writeFile(path_buffer->data, buffer->data, buffer->size);
    path_buffer->size = 0;
    buffer->size = 0;
}

void decompressFile(const char *original_file, expandable *path_buffer,
                    expandable *buffer, char *file_data, size_t file_size, node *huffman_tree,
                    int move_to_tmp_folder) {
    decompress(buffer, file_data, file_size, huffman_tree);
    char *new_file_path = strdup(original_file);
    new_file_path[strlen(new_file_path) - 4] = 0;
    if (move_to_tmp_folder) {
        useTmpFolderForFile(path_buffer, new_file_path);
    } else {
        appendSequenceExpandable(path_buffer, new_file_path, strlen(new_file_path));
    }
    zeroUnusedExpandable(path_buffer);
    writeFile(path_buffer->data, buffer->data, buffer->size);
    free(new_file_path);
    path_buffer->size = 0;
    buffer->size = 0;
}

expandablePtr *nodes;

void bst_dumper(const void *cur_node, const VISIT which, const int depth) {
    switch (which) {
        case preorder:
            break;
        case endorder:
            break;
        case leaf:
        case postorder:
            appendExpandablePtr(nodes, *((node **) cur_node));
            break;
    }
}

void buildHuffmanTreeFromBSTree(void **BSTree) {
    //sort counters
    nodes = createExpandablePtr();
    twalk(*BSTree, bst_dumper);
    qsort(nodes->data, (unsigned int) (nodes->size), sizeof(node *), qsort_cmp);
    if (((node *) (nodes->data[0]))->codes != NULL) {
        buildHuffmanTreeFromCodes((node **) (nodes->data), nodes->size);
        destroyExpandablePtr(nodes);
    } else {
        buildHuffmanTreeFromNodesArray((node **) (nodes->data), nodes->size);
        destroyExpandablePtrWithoutFree(nodes);
    }
}

void undoShits(const char *dir, const char *codebook_path) {
    char *command, *task_data, *line, *file_data;
    size_t task_size, file_size;
    void *BSTree = NULL;
    
    expandable *path_buffer = createExpandable();
    expandable *buffer = createExpandable();
    
    loadBSTFromCodeBookFile(codebook_path, &BSTree);
    buildHuffmanTreeFromBSTree(&BSTree);
    printf("Finished loading codebook\n");
    
    asprintf(&command, "find \"%s\" -type f -name \"*.hcz\" > output.tmp", dir);
    system(command);
    free(command);
    readFile("output.tmp", &task_data, &task_size);
    
    line = strtok(task_data, "\n");
    if(!line) {
        printf("Requested directory is empty\n");
        return;
    }
    while (line) {
        printf("Decompressing: %s\n", line);
        readFile(line, &file_data, &file_size);
        decompressFile(line, path_buffer, buffer, file_data, file_size, BSTree, 0);
        free(file_data);
        line = strtok(NULL, "\n");
    }
    
    destroyExpandable(path_buffer);
    destroyExpandable(buffer);
    free(task_data);
}

void undoSingleShit(const char *file, const char *codebook_path) {
    void *BSTree = NULL;
    expandable *buffer = createExpandable();
    expandable *path_buffer = createExpandable();
    char *file_data;
    size_t file_size;
    
    loadBSTFromCodeBookFile(codebook_path, &BSTree);
    buildHuffmanTreeFromBSTree(&BSTree);
    
    printf("Finished loading codebook\n");
    readFile(file, &file_data, &file_size);
    printf("Decompressing: %s\n", file);
    decompressFile(file, path_buffer, buffer, file_data, file_size, tree, 0);    //do not use tmp folder
    
    destroyExpandable(buffer);
    destroyExpandable(path_buffer);
    free(file_data);
}

void doShits(const char *dir, int has_codebook, const char *codebook_path, int generate_only) {
    char *command, *task_data, *line, *file_data;
    size_t task_size, file_size, items_count;

    void *BSTree = NULL;

    expandable **codes;
    expandable **words;


    asprintf(&command, "find \"%s\" -type f -name \"*.txt\" > output.tmp", dir);
    system(command);
    free(command);
    readFile("output.tmp", &task_data, &task_size);

    if (!has_codebook) {
        //need to count and build tree first
        char *task_data_dup = strdup(task_data);
        line = strtok(task_data_dup, "\n");
        if(!line) {
            printf("Requested directory is empty\n");
            return;
        }
        while (line) {
            printf("Building: %s\n", line);
            readFile(line, &file_data, &file_size);
            counting(file_data, file_size, &BSTree);
            free(file_data);
            line = strtok(NULL, "\n");
        }

        free(task_data_dup);

        //create huffman tree
        //sort counters
        buildHuffmanTreeFromBSTree(&BSTree);
        loadCodeBookFromTree(&codes, &words, size);
        items_count = size;

        dumpCodeBookToFileRaw(codebook_path, codes, words, items_count);

        //build codebook only
        if (generate_only) {
            cleanHalfCodeBooks(codes, words, items_count);
            return;
        } else {
            cleanHalfCodeBooks(codes, words, items_count);
        }
    } else {
        //use existed codebook
        loadBSTFromCodeBookFile(codebook_path, &BSTree);
        //buildHuffmanTreeFromBSTree(&BSTree);
        //exportCodeFromHuffmanTree(NULL, NULL);
        items_count = size;
    }
    
    printf("Finished loading codebook\n");
    
    line = strtok(task_data, "\n");

    expandable *output_buffer = createExpandable();
    expandable *output_path = createExpandable();
    while (line) {
        printf("Compressing: %s\n", line);
        readFile(line, &file_data, &file_size);
        compressFile(line, output_path, output_buffer, file_data, file_size, &BSTree, 0);
        free(file_data);
        line = strtok(NULL, "\n");
    }

    free(task_data);
    destroyExpandable(output_path);
    destroyExpandable(output_buffer);
}

void doSingleShit(const char *filepath, int has_codebook, const char *codebook_path, int generate_only) {
    char *file_data;
    expandable **codes;
    expandable **words;
    size_t file_size;
    int items_count;
    void *BSTree = NULL;
    readFile(filepath, &file_data, &file_size);
    if (!has_codebook) {
        counting(file_data, file_size, &BSTree);
        buildHuffmanTreeFromBSTree(&BSTree);
        // buildHuffmanTreeFromCounters(&counters);
        loadCodeBookFromTree(&codes, &words, size);
        items_count = size;

        //dump codebook if we dont have one yet
        dumpCodeBookToFileRaw(codebook_path, codes, words, items_count);

        //build codebook only
        if (generate_only) {
            cleanHalfCodeBooks(codes, words, items_count);
            return;
        } else {
            cleanHalfCodeBooks(codes, words, items_count);
        }
    } else {
        //use existed codebook
        loadBSTFromCodeBookFile(codebook_path, &BSTree);
        //buildHuffmanTreeFromBSTree(&BSTree);
        //exportCodeFromHuffmanTree(NULL, NULL);    //fill codes into tree
        items_count = size;
    }
    
    printf("Finished loading codebook\n");

    expandable *output_buffer = createExpandable();
    expandable *output_path = createExpandable();

    printf("Compressing: %s\n", filepath);
    compressFile(filepath, output_path, output_buffer, file_data, file_size, &BSTree, 0);
    free(file_data);

    destroyExpandable(output_path);
    destroyExpandable(output_buffer);
}
