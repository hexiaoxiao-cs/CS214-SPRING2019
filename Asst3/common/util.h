#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <libtar.h>

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


/*
 *  =============================================
 *
 *  EXPANDABLE BUFFER UTILITY
 *
 *  =============================================
 */

buffer* createBuffer();
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
void    appendBuffer(buffer *space, char c);

/*
 *  =============================================
 *
 *  FILE SYSTEM UTILITY
 *
 *  =============================================
 */

int     isDir(const char *name);
int     writeFile(const char *file_path, char *data, size_t size);
int     readFile(char *filename, char** buffer,size_t *size);
/*
 * Utility to sanitize input path (relative)
 *
 *
 * Return value:
 *  NULL: input path is not valid, this can be the result of multiple reasons
 *          1. the input_path is resolved to a location that is outside of our cwd
 *          2. the input_path is resolved to the cwd
 *
 *  Valid pointer: the sanitized version of input path (through malloc)
 *          Note:
 *          0. the pointer need to be freed using [free] after done using
 *          1. return has a valid pointer does not guarantee the existence of file/directory
 *          2. the escaped format does not contain '/' at the end (regardless whether the path is a directory)
 *          3. when one or more component directory of the referring file does not exist or such component directory is a file,
 *             this function will return the first directory that we cannot descend into
 *              Ex:
 *                  directory tree:
 *                      CWD
 *                       |---- abc (dir)
 *                       |         |------ def
 *                       |
 *                       \---- cba
 *
 *                  sanitize_path('./abc/def/gkk/cfk') = 'abc/def'
 *                      reason:
 *                          because def is a file (instead of a dir), we can't descend into such file, so we stop our further search
 *
 *                  sanitize_path('./ddd/ggg') = 'ddd'
 *                      reason:
 *                          because ddd directory does not exist in our CWD, so we stop our further descend
 *
 *                  sanitize_path('./abc/def') - 'abc/def'
 *                      reason:
 *                          this should be fairly straight forward
 *
 * Suggest usage:
 *  after sanitize_path, try to open the returned path
 *      if open success, it means the referring file is valid
 *      if the open failed, it means the referring file is invalid
 *          reason:
 *              if open failed means the referring file does not exist or one or more component directories does not exist
 *              if open success, either means the referring file exists, or one of the component directories is actually a file:
 *                  say we only have one file 'ttt', the path 'ttt/abc/def' still refers to ttt
 *
 *  Ex:
 *      '../${cwd}/abc/' -> 'abc'
 *      '../${cwd}/def/ggg/ccc' -> 'def/ggg/ccc'
 */
char* sanitize_path(const char* input_path);

/*
 *  This function utilizes sanitize_path to make sure the input_path is a valid path
 *  valid is defined as an openable file under current CWD
 *
 *  Implementation:
 *      this function try to open input_path before calling sanitize_path, if open failed return null, other wise return sanitizing result
 *
 *  Reason:
 *      if open success, it means the file exist after resolving by open system call,
 *      this branch eliminates all other possibilities which sanitize_path will return a valid pointer except our expected result
 *
 *  return value: see sanitize_path
 */
char* is_valid_path(const char* input_path);


/*
 *  =============================================
 *
 *  MANIFEST PROCESSING UTILITY
 *
 *  =============================================
 */

int readManifest(char* manifest_raw,size_t size, project* curr_project);
int writeManifest(char** manifest_towrite,project *curr_project,int old_new);
void sort_manifest(manifest_item** items, size_t len);
int compareManifest(int isTwoManifest, manifest_item** client_side, manifest_item** server_side, manifest_item*** changelog,manifest_item*** conflicts, size_t size_client, size_t size_server, int client_ver, int server_ver);
int proecessManifest_ByChangelist_Push(project* manifest,manifest_item** changelist, size_t changelist_size);
int proecessManifest_ByChangelist_Update(project* manifest,manifest_item** changelist, size_t changelist_size);
int readChangeLogFile(manifest_item ***changelog,char **input,size_t size, size_t *list_size);
int writeChangeLogFile(manifest_item **changelog,char** output,size_t size,int type);


/*
 *  =============================================
 *
 *  ENCODING UTILITY
 *
 *  =============================================
 */

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);
char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

/*
 *  =============================================
 *
 *  TAR EXTRACTION UTILITY
 *
 *  =============================================
 */

int tar_extract_specific_file(const char* tar_file, const char* stored_filename,
                              const char* output_root_path);

#endif