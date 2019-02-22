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
                    int* size       /* OUT: current block size in bytes */
                    );

void    write_header(    void* data,     /* INOUT: location for the header to be written in */
                        char is_used,    /* IN: 0 or 1 indicating whether the current block is occupied */
                        int size        /* IN: current block size in bytes [can be -1 indicating don't change the current block's size */
                        );

//return the location for the next header, NULL if not found
void*   get_next_header(void* current_header /* IN: location for the current header*/);