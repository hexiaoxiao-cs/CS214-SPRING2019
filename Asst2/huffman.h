#include "ds.h"
// typedef struct {
//   expandable* token;
//   int frequencies;
//   expandable* codes;
// } tnode;

//Following functions create BST files

//tnode->codes will be NULL when passed in
void buildHuffmanTreeFromNodesArray(node** nodes, int size); // sort and call buildHuffmanTreeFromFrequencies

void loadBSTFromCodeBookFile(const char* codebook_path, void** BSTree);
void counting(const char* file_data, int file_size, void* BSTree);
void compress(expandable* buffer, char* file_data, int file_size, void* BSTree);
void decompress(expandable* buffer, char* file_data, int file_size, node* huffman_tree);
void compressFile(const char* original_file, expandable* path_buffer, expandable* buffer, char* file_data, int file_size,
  void* BSTree, int move_to_tmp_folder);
void decompressFile(const char* original_file, expandable* path_buffer, expandable* buffer, char* file_data, int file_size, node* huffman_tree, int move_to_tmp_folder)
