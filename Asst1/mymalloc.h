/*
 * mymalloc.h
 * 
 * Declarations for mymalloc functions and replace system malloc
 */

#define malloc(x) mymalloc(c)
#define free(x) myfree(x)

//Interface
void*   mymalloc(int size);
void    myfree(void* input);

//Internal
void    readHeader( void* data,     /* IN: location for the header to read */
                    char* isUsed,   /* OUT: 0 or 1 indicating whether the current block is occupied */
                    int* size       /* OUT: current block size in bytes */
                    );

void    writeHeader(    void* data,     /* INOUT: location for the header to be written in */
                        char isUsed,    /* IN: 0 or 1 indicating whether the current block is occupied */
                        int size        /* IN: current block size in bytes [can be -1 indicating don't change the current block's size */
                        );

//return the location for the next header, NULL if not found
void*   getNextHeader(void* currentHeader /* IN: location for the current header*/);