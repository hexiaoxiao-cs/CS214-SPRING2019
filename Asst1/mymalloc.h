/*
 * mymalloc.h
 *
 * Declarations for mymalloc functions and replace system malloc
 */

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)

//Interface
void*   mymalloc(int size, const char*, int);
void    myfree(void* input, const char*, int);

//Internal

void    read_header(void* data,     /* IN: location for the header to read */
                    char* is_used,   /* OUT: 0 or 1 indicating whether the current block is occupied */
                    char* is_large,   /* OUT: 0 or 1 indicating whether the current header is large or not */
                    int* size       /* OUT: current block size in bytes */
);

//return the block size (header's size included) of which header is header
int     get_blk_size(void* header);

//adjust the header's size to a new_size and modified is_used flag. 
//IMPORTANT: This function will adjust new_size value as needed(THIS MEANS the ACTUAL new_size MAY NOT BE THE SAME AS INPUT)
//if new_size is smaller than the original size(shrinking), return value would be bytes that is not used immediately after the current block
//if new_size >= original size, return value is undefined
int     adjust_header_size(void* data, int new_size, char is_used);

//get the current data_pointer that can be used to return to the user
void*   get_data_pointer(void* header);

//return the location for the next header, NULL if not found
//IMPORTANT: returned locatoin might not contain a valid header
void*   get_next_header_position(void* current_header /* IN: location for the current header */);

//return the location for the previous header, NULL if not found
//IMPORTANT: returned location is guaranteed to have a valid header of not null
void*   get_previous_header(void* current_header /* IN: location for the current header */);
