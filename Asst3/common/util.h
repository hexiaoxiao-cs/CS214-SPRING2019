#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

// DEBUG UTIL

// DEBUG TOGGLE
#define DEBUG 1

#define TRACE(x) do { if (DEBUG) dbg_printf x; } while (0)

void dbg_printf(const char* fmt, ...);

typedef struct {
    char* data;
    size_t size;
    size_t total_size;
} buffer;

typedef struct{
    buffer* filename_64;
    buffer* filename;
    buffer* hash;
    buffer* newhash;
    long version_num;
    int changecode; // U-> 1, M->2, A->3, D->4
} manifest_item;

typedef struct{
    buffer* project_name;
    manifest_item **manifestItem; //manifestItem Matrix
    int project_version;
    int many_Items;
} project;

buffer* createBuffer();
int isDir(const char *name);
void    destroyBuffer(buffer* space);
void    destroyBufferWithoutFree(buffer* space);
void    expandBuffer(buffer* space, size_t size);
char*   peakBuffer(buffer* ptr);
size_t  availableBuffer(buffer* ptr);
char*   lastposBuffer(buffer* ptr);
void    copyoutBuffer(buffer* ptr, char* data, size_t size);
size_t  getLengthBuffer(buffer *ptr);
void    appendSequenceBuffer(buffer* space, const char* sequence, size_t sequence_size);
buffer* duplicateBuffer(buffer* space);
void    zeroUnusedBuffer(buffer* space);
int writeFile(const char *file_path, char *data, size_t size);
int readFile(char *filename, char** buffer,size_t *size);
int readManifest(char* manifest_raw,size_t size, project* curr_project);
int writeManifest(char** manifest_towrite,project *curr_project,int old_new);
void sort_manifest(manifest_item** items, size_t len);
int compareManifest(int isTwoManifest, manifest_item** client_side, manifest_item** server_side, manifest_item*** changelog,manifest_item*** conflicts, size_t size_client, size_t size_server, int client_ver, int server_ver);
int proecessManifest_ByChangelist_Push(project* manifest,manifest_item** changelist, size_t changelist_size);
int proecessManifest_ByChangelist_Update(project* manifest,manifest_item** changelist, size_t changelist_size);
unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);
char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);
#endif