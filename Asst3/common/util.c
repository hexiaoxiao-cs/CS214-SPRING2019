#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
//#include <stdlib.h>

//#include <lzma.h>
//DongFengDaoDan
//int compress(char** file_paths,char* path_7z)



//for Base64 Encrypting and Decrypring
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

int writeFile(const char *file_path, char *data, size_t size) {
    int handler = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (handler < 0) {
        //Unable to open specific files
        printf("Unable to open file %s\n", file_path);
    }
    //Blocking and writeAll
    size_t tmp;
    ssize_t ret;
    tmp = 0;
    while (tmp < size) {
        ret = write(handler, data + tmp, size - tmp);
        if (ret < 0) {
            return -1;
        } else if (ret == 0) {
            return -1;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    close(handler);    //Close file
    return 0;
}

int isDir(const char *name) {
    DIR *directory = opendir(name);

    if (directory == NULL) {
        return -1;
    }
    closedir(directory);
    return 0;
}


void build_decoding_table() {

    decoding_table = malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                          + (sextet_b << 2 * 6)
                          + (sextet_c << 1 * 6)
                          + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

void base64_cleanup() {
    free(decoding_table);
}

int readFile(char *filename, char** buffer,size_t *size){
    int fno=open(filename,O_RDONLY);
    if(fno<0){return -1;}
    //readAll
    size_t tmp;
    ssize_t ret;
    size_t file_size= (size_t) lseek(fno, 0,SEEK_END);
    lseek(fno,0,SEEK_SET);
    char *data = (char*) malloc(file_size+1);
    data[file_size]=0;
    tmp=0;
    while(tmp<file_size){
        ret = read(fno,data+tmp,file_size-tmp);
        if(ret<0){return -1;}
        else  if(ret==0){break;}
        else{
            tmp+=ret;
        }
    }
    *buffer=data;
    *size=file_size;
}


void dbg_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

buffer *createBuffer() {
    buffer *space = calloc(1, sizeof(buffer));
    space->data = calloc(1, 50 + 1);  //50 bytes, last bytes for NULL-terminator
    space->total_size = 50;
    return space;
}

void destroyBuffer(buffer *space) {
    free(space->data);
    free(space);
}
void destroyBufferWithoutFree(buffer *space) {
    free(space);
}

void expandBuffer(buffer *space, size_t size) {
    if (size <= 0)
        return;
    space->total_size = space->total_size + size;
    void *tmp = realloc(space->data, space->total_size + 1);
    if(tmp == NULL) {
        printf("Unable to allocate %ld bytes of memory\n", space->total_size + 1);
        exit(1);
    }
    space->data = tmp;
}

//void appendExpandable(buffer *space, char c) {
//    space->data[space->size++] = c;
//    if (space->size == space->total_size) {
//        if (space->size <= 1024)
//            expandExpandable(space, 10);  //expand 10 bytes each time for small memory
//        else
//            expandExpandable(space, space->size); //double the size each time for large memory
//    }
//    space->data[space->size] = 0;   //set the next byte to be 0
//}

/*
 * We do not need a remove buffer here
 * because our protocol definition is one packet only per connection, so there is no need to reuse connection to process multiple packets
 */

char* peakBuffer(buffer* ptr){
    return ptr->data;
}

size_t availableBuffer(buffer* ptr) {
    return ptr->total_size - ptr->size;
}

char* lastposBuffer(buffer* ptr) {
    return ptr->data + ptr->size;
}

void copyoutBuffer(buffer *ptr, char* data, size_t size) {
    memcpy(data, ptr->data, size);
}

size_t getLengthBuffer(buffer *ptr){
    return ptr->size;
}

void appendSequenceBuffer(buffer *space, const char *sequence, size_t sequence_size) {
    size_t available = space->total_size - space->size; //precalculate available size
    if (available <= sequence_size) {
        if (space->size + sequence_size > 1024) {
            expandBuffer(space, (sequence_size - available) + 4096000); //expand 4M each time for large memory
        } else {
            expandBuffer(space, sequence_size - available + 10);  //expand 10 bytes each time for small memory
        }
    }

    memcpy(space->data + space->size, sequence, sequence_size);
    space->size += sequence_size;
    space->data[space->size] = 0; //set the next byte to be 0
}

buffer* duplicateBuffer(buffer* space) {
    buffer* new_buffer = (buffer*)malloc(sizeof(buffer));
    new_buffer->size = space->size;
    new_buffer->total_size = space->total_size;
    new_buffer->data = (char*)malloc(new_buffer->total_size + 1);   //add one here because we need space for the null terminator
    memcpy(new_buffer->data, space->data, new_buffer->size + 1);    // add one here because buffer is guaranteed null terminated
    return new_buffer;
}

void zeroUnusedBuffer(buffer *space) {
    memset(space->data + space->size, 0, space->total_size - space->size);  //zero out unused space
}

//  Manifest Format:
//  Made by blah blah blah
//  Filename with path in base64	file_Version#	hash#
//  outside malloc curr_project, project name need to be written in that struct

int readManifest(char* manifest_raw,size_t size, project* curr_project){
    //char* manifest_raw;
    char* kk;
    buffer *temporary;
    int status,tmp=0;
    int type=0,count =0 ;
    size_t tt=0;
    manifest_item *curr;
    //if(status!=0){return -1;}
    temporary=createBuffer();
    for(tmp=16;tmp<size;tmp++){
        if(manifest_raw[tmp]=='\n' || manifest_raw[tmp]==' '){
            if(type == 0 ){
                curr=malloc(sizeof(manifest_item));
                curr->filename_64=temporary;
                temporary=createBuffer();
                kk=base64_decode(curr->filename_64->data,curr->filename_64->size,&tt);
                appendSequenceBuffer(temporary,kk,tt);
                free(kk);
                curr->filename=temporary;
                temporary=createBuffer();
                //free(temporary);
                //destroyBufferWithoutFree(temporary);
                type++;
            }
            else{
                if(type == 1){
                    curr=malloc(sizeof(manifest_item));
                    curr->version_num=atol(temporary->data);
                    //free(temporary);
                    //destroyBufferWithoutFree(temporary);
                    temporary=createBuffer();
                    type++;
                }
                else{
                    if(type == 2){
                        curr=malloc(sizeof(manifest_item));
                        curr->hash=temporary;
                        //free(temporary);
                        //destroyBufferWithoutFree(temporary);
                        temporary=createBuffer();
                        type=0;
                        curr_project->manifestItem=(manifest_item**)realloc(curr_project->manifestItem,sizeof(manifest_item*)*(count+1));
                        //if(status!=0){return -2;}
                        curr_project->manifestItem[count]=curr;
                        count++;
                    }
                }
            }
        }
        else{
            appendSequenceBuffer(temporary,manifest_raw,tmp);
        }
    }
    if(type!=0){return -1;}
    curr_project->many_Items=count;
    return 0;
}
//Count is how many stuff you have in the manifest_item (index+1)

int writeManifest(char** manifest_towrite,project *curr_project,int old_new){
    char* temp;
    int tmp=0;
    *manifest_towrite=(char*) malloc(sizeof(char)*17);
    strcpy(*manifest_towrite,"Made_By_HXX&DZZ\n");
    for(tmp=0;tmp<curr_project->many_Items;tmp++){
        if(old_new==0) {
            asprintf(&temp, "%s %ld %s\n", curr_project->manifestItem[tmp]->filename_64->data,
                     curr_project->manifestItem[tmp]->version_num, curr_project->manifestItem[tmp]->hash->data);
        }
        else{
            asprintf(&temp, "%s %ld %s\n", curr_project->manifestItem[tmp]->filename_64->data,
                     curr_project->manifestItem[tmp]->version_num, curr_project->manifestItem[tmp]->newhash->data);
        }
        *manifest_towrite=(char*) realloc(*manifest_towrite,strlen(*manifest_towrite)+strlen(temp)+1);
        *manifest_towrite=strcat(*manifest_towrite,temp);
    }
    return 0;
}

