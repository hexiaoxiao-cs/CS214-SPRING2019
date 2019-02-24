/*
 * mymalloc.h
 *
 * Declarations for mymalloc functions and replace system malloc
 */

#define malloc(x) mymalloc(x)
#define free(x) myfree(x)

//Interface
void*   mymalloc(int size);
void    myfree(void* input);

//Internal

void    read_header( void* data,     /* IN: location for the header to read */
                    char* is_used,   /* OUT: 0 or 1 indicating whether the current block is occupied */
		    char* is_large,   /* OUT: 0 or 1 indicating whether the current header is large or not */
                    int* size       /* OUT: current block size in bytes */
                    );

int get_blk_size(void* header);
int adjust_header_size(void* data, int new_size, char is_used);
void* get_data_pointer(void* header);

//return the location for the next header, NULL if not found
void*   get_next_header_position(void* current_header /* IN: location for the current header */);

//return the location for the previous header, NULL if not found
void*   get_previous_header(void* current_header /* IN: location for the current header */);
