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
#include <util.h>
#include <libtar.h>
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

int writeFile(const char *file_path, const char *data, size_t size) {
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
    int i;
    decoding_table = malloc(256);

    for (i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {
    int i, j;
    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char) data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char) data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char) data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {
    int i, j;
    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (i = 0, j = 0; i < input_length;) {

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

int readFile(char *filename, char **buffer, size_t *size) {
    int fno = open(filename, O_RDONLY);
    if (fno < 0) { return -1; }
    //readAll
    size_t tmp;
    ssize_t ret;
    size_t file_size = (size_t) lseek(fno, 0, SEEK_END);
    lseek(fno, 0, SEEK_SET);
    char *data = (char *) malloc(file_size + 1);
    data[file_size] = 0;
    tmp = 0;
    while (tmp < file_size) {
        ret = read(fno, data + tmp, file_size - tmp);
        if (ret < 0) { return -1; }
        else if (ret == 0) { break; }
        else {
            tmp += ret;
        }
    }
    *buffer = data;
    *size = file_size;
    return 0;
}


void dbg_printf(const char *fmt, ...) {
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
    if (tmp == NULL) {
        printf("Unable to allocate %ld bytes of memory\n", space->total_size + 1);
        exit(1);
    }
    space->data = tmp;
}

void appendBuffer(buffer *space, char c) {
    space->data[space->size++] = c;
    if (space->size == space->total_size) {
        if (space->size <= 1024)
            expandBuffer(space, 10);  //expand 10 bytes each time for small memory
        else
            expandBuffer(space, space->size); //double the size each time for large memory
    }
    space->data[space->size] = 0;   //set the next byte to be 0
}

/*
 * We do not need a remove buffer here
 * because our protocol definition is one packet only per connection, so there is no need to reuse connection to process multiple packets
 */

char *peakBuffer(buffer *ptr) {
    return ptr->data;
}

size_t availableBuffer(buffer *ptr) {
    return ptr->total_size - ptr->size;
}

char *lastposBuffer(buffer *ptr) {
    return ptr->data + ptr->size;
}

void copyoutBuffer(buffer *ptr, char *data, size_t size) {
    memcpy(data, ptr->data, size);
}

size_t getLengthBuffer(buffer *ptr) {
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

buffer *duplicateBuffer(buffer *space) {
    buffer *new_buffer = (buffer *) malloc(sizeof(buffer));
    new_buffer->size = space->size;
    new_buffer->total_size = space->total_size;
    new_buffer->data = (char *) malloc(
            new_buffer->total_size + 1);   //add one here because we need space for the null terminator
    memcpy(new_buffer->data, space->data,
           new_buffer->size + 1);    // add one here because buffer is guaranteed null terminated
    return new_buffer;
}

void zeroUnusedBuffer(buffer *space) {
    memset(space->data + space->size, 0, space->total_size - space->size);  //zero out unused space
}

/*
 * output_root_path has to have a '/' as it's last character
 *
 * This function extract a specific file inside tar_file.
 * The specific file is specified by stored_filename
 *
 * output_root_path indicates the path extraction will output
 */
int tar_extract_specific_file(const char *tar_file, const char *stored_filename,
                              const char *output_root_path) {
    TAR *tar;
    int ret;

    char *out_path = (char *) malloc(strlen(output_root_path) + strlen(stored_filename) + 1);
    char *appender = out_path + strlen(output_root_path);

    strcpy(out_path, output_root_path);

    if (tar_open(&tar, (char *) tar_file, NULL, O_RDONLY, 0700, TAR_GNU) < 0)
        return -1;  // Unable to open file

    while ((ret = th_read(tar)) == 0) {
        if (strcmp(th_get_pathname(tar), stored_filename) == 0) {
            strcpy(appender, stored_filename);
            tar_extract_file(tar, out_path);
            tar_close(tar);
            free(out_path);
            return 0;
        }
        if (TH_ISREG(tar))
            tar_skip_regfile(tar);
    }

    tar_close(tar);

    free(out_path);

    return -2;
}

char *is_valid_path(const char *input_path, const char *project_name) {
    char *sanitized_path;
    int op_ret;
    if ((op_ret = open(input_path, O_RDONLY)) < 0)
        return NULL;
    close(op_ret);
    return sanitize_path(input_path, project_name);
}

//TODO: sanitize_path add project name input, remove project name from relative

char *sanitize_path(const char *input_path, const char *project_name) {
    char cwd[PATH_MAX];
    char real_path[PATH_MAX];
    char *cursor;
    char *output;
    getcwd(cwd, PATH_MAX);
    strcat(cwd, "/");
    strcat(cwd, project_name);
    realpath(input_path, real_path);
    cursor = strstr(real_path, cwd);
    if (!cursor) {
        return NULL;
    }
    if (cursor != real_path) {
        return NULL;    // there is a cwd somewhere in between
    }
    if (cursor[strlen(cwd)] == 0) {
        return NULL;    // it only contain cwd
    }
    output = (char *) malloc(strlen(real_path) - strlen(cwd) - 1 + 1);   // -1 is for the '/'
    strcpy(output, cursor + strlen(cwd) + 1);
    return output;
}

//  Manifest Format:
//  Made by blah blah blah
//  Filename with path in base64	file_Version#	hash#
//  outside malloc curr_project, project name need to be written in that struct

int readManifest(const char *manifest_raw, size_t size, project *curr_project) {
    //char* manifest_raw;
    char *kk;
    buffer *temporary;
    int status, tmp = 0;
    int type = 0, count = 0;
    int read = 0;
    size_t tt = 0;
    manifest_item *curr;
    //if(status!=0){return -1;}
    temporary = createBuffer();
    tmp = 16;
    sscanf(manifest_raw + tmp, "%d\n%n", &(curr_project->project_version), &read);
    tmp += read;
    curr_project->manifestItem = (manifest_item **) malloc(1 * sizeof(manifest_item *));
    for (; tmp < size; tmp++) {
        if (manifest_raw[tmp] == '\n' || manifest_raw[tmp] == ' ') {
            if (type == 0) {
                curr = malloc(sizeof(manifest_item));
                curr->filename_64 = temporary;
                temporary = createBuffer();
                kk = base64_decode(curr->filename_64->data, curr->filename_64->size, &tt);
                appendSequenceBuffer(temporary, kk, tt);
                free(kk);
                curr->filename = temporary;
                temporary = createBuffer();
                //free(temporary);
                //destroyBufferWithoutFree(temporary);
                type++;
            } else {
                if (type == 1) {
                    curr->version_num = atol(temporary->data);
                    //free(temporary);
                    //destroyBufferWithoutFree(temporary);
                    temporary = createBuffer();
                    type++;
                } else {
                    if (type == 2) {
                        curr->hash = temporary;
                        //free(temporary);
                        //destroyBufferWithoutFree(temporary);
                        temporary = createBuffer();
                        type = 0;
                        curr_project->manifestItem = (manifest_item **) realloc(curr_project->manifestItem,
                                                                                sizeof(manifest_item *) * (count + 1));
                        //if(status!=0){return -2;}
                        curr_project->manifestItem[count] = curr;
                        count++;
                    }
                }
            }
        } else {
            appendBuffer(temporary, manifest_raw[tmp]);
        }
    }
    if (type != 0) { return -1; }
    curr_project->many_Items = count;
    return 0;
}

//Count is how many stuff you have in the manifest_item (index+1)
//old -> 0 new ->1
int writeManifest(char **manifest_towrite, project *curr_project, int old_new) {
    char *temp;
    int tmp = 0;
    *manifest_towrite = (char *) malloc(sizeof(char) * 30);
    strcpy(*manifest_towrite, "Made_By_HXX&DZZ\n");
    asprintf(&temp, "%d\n", curr_project->project_version);
    *manifest_towrite = (char *) realloc(*manifest_towrite, strlen(*manifest_towrite) + strlen(temp) + 1);
    *manifest_towrite = strcat(*manifest_towrite, temp);
    for (tmp = 0; tmp < curr_project->many_Items; tmp++) {
        if (old_new == 0) {
            asprintf(&temp, "%s %ld %s\n", curr_project->manifestItem[tmp]->filename_64->data,
                     curr_project->manifestItem[tmp]->version_num, curr_project->manifestItem[tmp]->hash->data);
        } else {
            asprintf(&temp, "%s %ld %s\n", curr_project->manifestItem[tmp]->filename_64->data,
                     curr_project->manifestItem[tmp]->version_num, curr_project->manifestItem[tmp]->newhash->data);
        }
        *manifest_towrite = (char *) realloc(*manifest_towrite, strlen(*manifest_towrite) + strlen(temp) + 1);
        *manifest_towrite = strcat(*manifest_towrite, temp);
    }
    return 0;
}

int cmp_compare(const void *a_, const void *b_) {
    manifest_item *a = (manifest_item *) a_;
    manifest_item *b = (manifest_item *) b_;
    if (a->filename->size != b->filename->size) {
        return a->filename->size - b->filename->size;
    } else {
        return memcmp(a->filename->data, b->filename->data, a->filename->size);
    }
}

int cmp_compare_qsort(const void *a_, const void *b_) {
    manifest_item *a = *((manifest_item **) a_);
    manifest_item *b = *((manifest_item **) b_);
    if (a->filename->size != b->filename->size) {
        return a->filename->size - b->filename->size;
    } else {
        return memcmp(a->filename->data, b->filename->data, a->filename->size);
    }
}

void sort_manifest(manifest_item **items, size_t len) {
    qsort(items, len, sizeof(manifest_item *), cmp_compare_qsort);
}
//            if(cmp_compare(client_side[curr_client],server_side[curr_server])>0){
//                if(server_ver!=client_ver){// server has something that clients does not have and the proj version number is different, therefore the changecode is D->4
//                    changelog=(manifest_item***)realloc(changelog,sizeof(manifest_item**)*(counts+1));
//                    (*changelog)[counts]=server_side[curr_server];
//                    (*changelog)[counts]->changecode=4;
//                    curr_server++;
//                    //continue;
//                }
//                else{
//                    // server has something that client does not have, and the version number of proj is the same, therefore the changecode is U->1
//                    changelog=(manifest_item***)realloc(changelog,sizeof(manifest_item**)*(counts+1));
//                    (*changelog)[counts]=server_side[curr_server];
//                    (*changelog)[counts]->changecode=1;
//                    curr_server++;
//                }
//            }
//            else if(cmp_compare(client_side[curr_client],server_side[curr_server])<0){
//                changelog=(manifest_item***)realloc(changelog,sizeof(manifest_item**)*(counts+1));
//                (*changelog)[counts]=client_side[curr_server];
//                (*changelog)[counts]->changecode=4; // client has something that server does not have, therefore the chagnecode is A->3
//                curr_server++;
//            }

// is two manifest -> 0 ->Two Manifest, 1-> One Manifest
// Client_side-> need to have both new hash and old hash,
// Server_side-> only compare old hash
// Changelog-> Any differences between those two files
// Any conflicts are exhibited in the conflicts pointer with an returned value of -9
int
compareManifest(int isTwoManifest, manifest_item **client_side, manifest_item **server_side, manifest_item ***changelog,
                manifest_item ***conflicts, size_t size_client, size_t size_server, int client_ver, int server_ver,
                size_t *changelog_size, size_t *conflicts_size, int is_commit) {
    size_t curr_client = 0, curr_server = 0;
    size_t counts = 0, counts_conflicts = 0;
    int has_conflicts = 0;
    (*changelog) = (manifest_item **) malloc(sizeof(manifest_item *));
    (*conflicts) = (manifest_item **) malloc(sizeof(manifest_item *));
    while (curr_client < size_client && curr_server < size_server) {
        if (isTwoManifest == 0) {
            if (cmp_compare(client_side[curr_client], server_side[curr_server]) >
                0) {//a file in the server but not in client side
                if (client_ver != server_ver || is_commit==1) {
                    *changelog = (manifest_item **) realloc((*changelog), sizeof(manifest_item *) * (counts + 1));
                    (*changelog)[counts] = server_side[curr_server];
                    (*changelog)[counts]->changecode = 3; // Something being added in the server side
                    curr_server++; // server to the next item
                    counts++;
                } else {
                    *conflicts = (manifest_item **) realloc(*conflicts,
                                                            sizeof(manifest_item *) * (counts_conflicts + 1));
                    (*conflicts)[counts_conflicts] = server_side[curr_server];
                    (*conflicts)[counts_conflicts]->changecode = 5; // In Server Not In Client Conflicts!!
                    curr_server++; // server to the next item
                    counts_conflicts++;
                    has_conflicts = 1;
                } // Manifest File Corrupted Or Conflicts!!!! Need to Do Something!!!!
            } else {
                if (cmp_compare(client_side[curr_client], server_side[curr_server]) <
                    0) {// a file in the client side but not in server side
                    if (client_ver == server_ver) {
                        *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                        (*changelog)[counts] = client_side[curr_client];
                        (*changelog)[counts]->changecode = 1; // Something needed to be upload in the server side
                        curr_client++; // server to the next item
                        counts++;
                    } else {
                        *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                        (*changelog)[counts] = client_side[curr_client];
                        (*changelog)[counts]->changecode = 4; // Something needed to be deleted in the client side
                        curr_client++; // server to the next item
                        counts++;
                    }
                } else {
                    if (cmp_compare(client_side[curr_client], server_side[curr_server]) ==
                        0) { // a file in both client and server side
                        if (client_ver == server_ver &&
                            strcmp(client_side[curr_client]->newhash->data, server_side[curr_client]->hash->data) !=
                            0) { // they have the same version number and not same hash (new hash from client and old has from server)
                            *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                            (*changelog)[counts] = client_side[curr_client];
                            (*changelog)[counts]->changecode = 1; // Something needed to be upload in the server side

                            curr_client++; // curr_client to the next item
                            curr_server++; // curr-server to the nect item
                            counts++;
                        } else {
                            if (client_side != server_side &&
                                strcmp(client_side[curr_client]->newhash->data, client_side[curr_client]->hash->data) ==
                                0) {// different version number but the file in client does not changed (Only) REMOTE CHANGED
                                *changelog = (manifest_item **) realloc(*changelog,
                                                                        sizeof(manifest_item *) * (counts + 1));
                                (*changelog)[counts] = server_side[curr_client];
                                (*changelog)[counts]->changecode = 2; // Modified by remote
                                (*changelog)[counts]->newhash = server_side[curr_server]->hash; // Modified by remote
                                curr_client++; // curr_client to the next item
                                curr_server++; // curr-server to the nect item
                                counts++;
                            } else {
                                *conflicts = (manifest_item **) realloc(*conflicts, sizeof(manifest_item *) *
                                                                                    (counts_conflicts + 1));
                                (*conflicts)[counts_conflicts] = server_side[curr_server];
                                (*conflicts)[counts_conflicts]->changecode = 6; // Something being added in the server side
                                curr_server++; // server to the next item
                                counts_conflicts++;
                                has_conflicts = 1;
                                //return -9; //Something Wrong or Conflict!!!
                            }

                        }
                    }
                }
            }
//            if (strcmp(client_side[curr_client]->hash->data, client_side[curr_client]->newhash->data) != 0) {
//                *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
//                (*changelog)[counts] = client_side[curr_client];
//                (*changelog)[counts]->changecode = 1; // Something needed to be upload in the server side
//                curr_client++; // curr_client to the next item
//                //curr_server++; // curr-server to the nect item
//                counts++;
//            } else {
//                curr_client++;
//            }
        }
        if (isTwoManifest == 1) { // one manifest -> client side
            if (strcmp(client_side[curr_client]->hash->data, client_side[curr_client]->newhash->data) != 0) {
                *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                (*changelog)[counts] = client_side[curr_client];
                (*changelog)[counts]->changecode = 1;
                counts++;

            }
            curr_client++;
        }
    }
    if (isTwoManifest == 0) {
        if (size_client > curr_client) { // client does not go to the end
            //client has something that server does not have
            for (; curr_client < size_client; curr_client++) {
// a file in the client side but not in server side
                if (client_ver == server_ver) {
                    *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                    (*changelog)[counts] = client_side[curr_client];
                    (*changelog)[counts]->changecode = 1; // Something needed to be upload in the server side
                    //curr_client++; // server to the next item
                    counts++;
                } else {
                    *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                    (*changelog)[counts] = client_side[curr_client];
                    (*changelog)[counts]->changecode = 4; // Something needed to be deleted in the client side
                    //curr_client++; // server to the next item
                    counts++;
                }
//                        *changelog = (manifest_item **) realloc(changelog, sizeof(manifest_item *) * (counts + 1));
//                        (*changelog)[counts] = client_side[curr_client];
//                        (*changelog)[counts]->changecode = 1; // Something needed to be upload in the server side
//                        //curr_client++; // server to the next item
//                        counts++;

            }
        } else {// server does not go to the end
            for (; curr_server < size_server; curr_server++) {
                if (client_ver != server_ver || is_commit==1) {
                    *changelog = (manifest_item **) realloc(*changelog, sizeof(manifest_item *) * (counts + 1));
                    (*changelog)[counts] = server_side[curr_server];
                    (*changelog)[counts]->changecode = 3; // Something being added in the server side
                    //curr_server++; // server to the next item
                    counts++;
                } else {
                    *conflicts = (manifest_item **) realloc(*conflicts,
                                                            sizeof(manifest_item *) * (counts_conflicts + 1));
                    (*conflicts)[counts_conflicts] = server_side[curr_server];
                    (*conflicts)[counts_conflicts]->changecode = 5; // In Server Not In Client Conflicts!!
                    //curr_server++; // server to the next item
                    counts_conflicts++;
                    has_conflicts = 1;
                } // Manifest File Corrupted Or Conflicts!!!! Need to Do Something!!!!

//                *changelog = (manifest_item **) realloc(changelog, sizeof(manifest_item *) * (counts + 1));
//                (*changelog)[counts] = server_side[curr_server];
//                (*changelog)[counts]->changecode = 3; // Something needed to be deleted in the server side
//                //curr_client++; // server to the next item
//                counts++;
            }
        }
    }

    if (has_conflicts == 1) {
        (*conflicts_size) = counts_conflicts;
        return -9;
    }
    (*changelog_size) = counts;
    return 0;
}
//Increment file version number according to the changelist
//Change Hash to the "current status" hash code
//Requirement: Both lists are sorted
//!!!!!This Is Only Used By Commit and Push

int proecessManifest_ByChangelist_Push(project *manifest, manifest_item **changelist, size_t changelist_size) {
    size_t temp = 0, m_size = 0, new_size = 0;
    manifest->project_version++;
    manifest_item **c = manifest->manifestItem;
    manifest_item **new_manifest = malloc(sizeof(manifest_item *) * (changelist_size + manifest->many_Items));

    for (temp = 0; temp < changelist_size; temp++) {
        if(changelist[temp]->changecode==3){continue;}
        while (m_size < manifest->many_Items ) {
            if(cmp_compare(c[m_size], changelist[temp]) < 0) {
                new_manifest[new_size] = c[m_size];
                new_manifest[new_size]->hash = new_manifest[new_size]->newhash;
                new_size++;
                m_size++;
            } else
                break;
        }
        if (cmp_compare(c[m_size], changelist[temp]) == 0) {
            if (changelist[temp]->changecode == 1) {
                // Upload to server, Modified by server
                // Update Hash
                new_manifest[new_size] = c[m_size];
                new_manifest[new_size]->hash = new_manifest[new_size]->newhash;
                new_manifest[new_size]->version_num++;
                new_size++;
                m_size++;
            } else {
                if (changelist[temp]->changecode == 3) {
                    //Deleted
                    //Free Memory
                    free(c[m_size]);
                    m_size++;
                    new_size++;
                }
            }

        } else { //filename are different ,which means we need to add the change list item to the result list
            if (changelist[temp]->changecode == 4) {
                new_manifest[new_size] = c[temp];
                new_size++;
            }
        }
    }
    manifest->manifestItem = new_manifest;
    free(c);
    free(changelist);
    manifest->many_Items = new_size;
    return 0;
}

//Input Server's Manifest

int proecessManifest_ByChangelist_Update(project *manifest, manifest_item **changelist, size_t changelist_size,
                                         project *server) {
    size_t temp = 0, m_size = 0, new_size = 0, s_size = 0;
    manifest->project_version++;
    manifest_item **c = manifest->manifestItem;
    manifest_item **s = server->manifestItem;
    manifest_item **new_manifest = malloc(sizeof(manifest_item *) * (changelist_size + manifest->many_Items));
    for (temp = 0; temp < changelist_size; temp++) {
        while (cmp_compare(c[m_size], changelist[temp]) < 0 && m_size < manifest->many_Items) {
            new_manifest[new_size] = c[m_size];
            new_manifest[new_size]->hash = new_manifest[new_size]->newhash;
            new_size++;
            m_size++;
        }
        while (cmp_compare(s[s_size], changelist[temp]) != 0) {
            s_size++;
            //potential segmentation fault
        }
        new_manifest[new_size] = s[s_size];
        new_size++;
//        if(changelist[temp]->changecode==1){
//            new_manifest[new_size]=changelist[temp];
//            //new_manifest[new_size]->hash=new_manifest[new_size]->newhash;
//            new_size++;
//        }
    }
    manifest->manifestItem = new_manifest;
    free(c);
    free(changelist);
    manifest->many_Items = new_size;
    return 0;
}
// TODO: Need to give project directory (DZZ)


//Changelog File Format:
//Made_By_HXX&DZZ
//Version Number (Upgrade Remote Version Number, )
//ChangeCode(char) Filename Filename_64 File_Version Hash_old Hash_new
//1->MAD 2->UAD 3->Conflicts

int writeChangeLogFile(manifest_item **changelog, char **output, size_t size, int type, long version) {
    size_t curr = 0;
    char *temporary;
    buffer *o;
    o = createBuffer();
    appendSequenceBuffer(o, "Made_By_HXX&DZZ\n", 16);
    asprintf(&temporary, "%ld\n", version);
    appendSequenceBuffer(o, temporary, strlen(temporary));
    for (curr = 0; curr < size; curr++) {
        if (type != 3) {
            if ((changelog[curr]->changecode == 1 && type != 1)) { // U
                asprintf(&temporary, "%d %s %s %ld %s %s\n", changelog[curr]->changecode,
                         changelog[curr]->filename->data,
                         changelog[curr]->filename_64->data, changelog[curr]->version_num, changelog[curr]->hash->data,
                         changelog[curr]->newhash->data);
            } else {
                if ((changelog[curr]->changecode == 2 && type != 2) || changelog[curr]->changecode == 4 ||
                    changelog[curr]->changecode == 3) {//M A D
                    asprintf(&temporary, "%d %s %s %ld %s 0\n", changelog[curr]->changecode,
                             changelog[curr]->filename->data,
                             changelog[curr]->filename_64->data, changelog[curr]->version_num,
                             changelog[curr]->hash->data);
                }
            }
        } else {
            if (changelog[curr]->changecode == 5 || changelog[curr]->changecode == 6) { // U
                asprintf(&temporary, "Conflicts:%s\n", changelog[curr]->filename->data);
            }

        }
        appendSequenceBuffer(o, temporary, strlen(temporary));
    }
    *output = o->data;
    return 0;
}

int readChangeLogFile(manifest_item ***changelog, char **input, size_t size, int *list_size, long *version) {
    manifest_item **temp = (manifest_item **) malloc(sizeof(manifest_item *));
    manifest_item *item;
    char *to_trans = *input, *tfilename, *tbase64, *tsha256, *tsha256_new;
    size_t read_bytes = 0, list = 0;
    to_trans += 16;
    size -= 16;
    sscanf(to_trans, "%ld\n%n", version, &read_bytes);
    to_trans += read_bytes;
    size -= read_bytes;
    read_bytes = 0;
    tfilename = (char *) malloc(100000);
    tbase64 = (char *) malloc(100000);
    tsha256 = (char *) malloc(100);
    tsha256_new = (char *) malloc(100);
    while (size != 0) {
        item = (manifest_item *) malloc(sizeof(manifest_item));
        item->filename_64 = createBuffer();
        item->filename = createBuffer();
        item->newhash = createBuffer();
        item->hash = createBuffer();
        sscanf(to_trans, "%d %s %s %ld %s %s\n%ln", &(item->changecode), tfilename, tbase64, &(item->version_num),
               tsha256, tsha256_new, &read_bytes); // %n is how many bytes i've read sofar
        to_trans += read_bytes;
        size -= read_bytes;
        read_bytes = 0;
        appendSequenceBuffer(item->filename, tfilename, strlen(tfilename));
        appendSequenceBuffer(item->filename_64, tbase64, strlen(tbase64));
        appendSequenceBuffer(item->hash, tsha256, strlen(tsha256));
        appendSequenceBuffer(item->newhash, tsha256_new, strlen(tsha256_new));
        temp = (manifest_item **) realloc(temp, sizeof(manifest_item *) * (list + 1));
        temp[list] = item;
        list++;
    }
    *list_size = list;
    *changelog = temp;
    return 0;
}

