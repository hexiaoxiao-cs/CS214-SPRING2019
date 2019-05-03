#include "client.h"
#include "util.h"
#include <protocol.h>
#include <network.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>

#define PARSEERROR "Argv Error\nUsage:\n./wtf [configure <IP/hostname> <port>] [checkout <project name>] [update <project name>]\n[upgrade <project name>] [commit <project name>] [push <project name>]\n[create <project name>] [destroy <project name>] [add <project name> <filename>]\n[remove <project name> <filename>] [currentversion <project name>] [history <project name>]\n[rollback <project name> <version>]\n"

char* ipaddr;
int portno;

//using openSSL hashing library

void sha256_string(const char* data, size_t len, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int i;
    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(hash, &sha256);

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

int sha256_file(char *path, char outputBuffer[65])
{
    // call sha256_string will be good
//
//    FILE *file = fopen(path, "rb");
//    if(!file) return -534;
//
//    unsigned char hash[SHA256_DIGEST_LENGTH];
//    SHA256_CTX sha256;
//    SHA256_Init(&sha256);
//    const int bufSize = 32768;
//    unsigned char *buffer = malloc(bufSize);
//    int bytesRead = 0;
//    if(!buffer) return ENOMEM;
//    while((bytesRead = fread(buffer, 1, bufSize, file)))
//    {
//        SHA256_Update(&sha256, buffer, bytesRead);
//    }
//    SHA256_Final(hash, &sha256);
//
//    sha256_hash_string(hash, outputBuffer);
//    fclose(file);
//    free(buffer);
    return 0;
}



//make new manifest according to the array listed files
//rehash
int make_new_manifest(manifest_item **array,int counts){
    int temp=0;
    char* file_data;
    int status;
    size_t size;
    char sha256buf[65];
    for(temp=0;temp<counts;temp++){
        status=readFile(array[temp]->filename->data,&file_data,&size);
        array[temp]->newhash=createBuffer();
        if(status!=0){ continue;}
        sha256_string(file_data,size,sha256buf);
        appendSequenceBuffer(array[temp]->newhash,sha256buf,65);
    }
    return 0;
}

int create(char* project_name){
    int op=0;
    buffer *output,*input;
    parsed_response_t response;
    output=get_output_buffer_for_request(op,project_name,strlen(project_name));
    send_request(output,&input);
    response=parse_response(input);
    if(strcmp(response.str_payload.payload,"OK")!=0){return 0;}
    else{return -1;}
}
int history(char* project_name){
    int op =1;
    char *the_history;
    buffer *output,*input;
    parsed_response_t response;
    output=get_output_buffer_for_request(op,project_name,strlen(project_name));
    send_request(output,&input);
    response=parse_response(input);
    if(response.str_payload.payload_size!=0){return -1;}
    printf("%s",response.str_payload.payload);
    return 0;
}

int current_version(char* project_name){
    int op=2;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));


    return 0;
}
int destroy(char* project_name){
    int op=3;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
}
int rollback(char* project_name){
    int op=4;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int checkout(char* project_name){
    int op=5;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int update(char* project_name){
    int op=6;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));

    return 0;
}
int upgrade(char* project_name){
    int op=7;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int commit(char* project_name){
    int op=8;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int push(char* project_name){
    int op = 9 ;
    buffer *output,*input;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;

}

//0 -> OK!
//-1-> File Not Found
//-2-> Is Directory

int add(char* project_name, char* path){

}

//read Configure under the current directory .configure file

int readConfigure (){
    char *buffer;
    size_t size;
    if(readFile("./.configure",&buffer,&size)==-1){return -1;}
    ipaddr=malloc(sizeof(char)*size);
    return sscanf(buffer,"%s\n%d",ipaddr,&portno);

}

int configure(char *server_addr,char* port_no){
    char* data;
    size_t size;
    size=asprintf(&data,"%s\n%s",server_addr,port_no);
    writeFile("./.configure",data,size);
    return 0;
}

int main(int argc,char* argv[]) {
    if(argc <2) {printf(PARSEERROR);return -1;}
    if(strlen(argv[1])<3){printf(PARSEERROR);}

    if ((argv[1][0] + argv[1][2] * 100) != 11099) {
        if (readConfigure() == -1) { printf("Error Happened During Reading Configure File\n"); }
    }

    switch(argv[1][0]+argv[1][2]*100){
        case 11099: {
            if (strcmp("configure", argv[1]) != 0 || argc==4) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("configure");
             configure(argv[2],argv[3]);
            break;
        }
        case 10199: {
            if (strcmp("checkout", argv[1]) != 0 || argc==3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("checkout");
            if(checkout(argv[3])!=0){printf("Error checkout project with name %s\nPossible Reasons:\n1. Project %s does not exist in sever.\n2. Error communicating with server.\n",argv[2],argv[2]);}
            break;
        }
        case 10117: {
            if (strcmp("update", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("update");
            break;
        }
        case 10417: {
            if (strcmp("upgrade", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("upgrade");
            break;
        }
        case 10999: {
            if (strcmp("commit", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("commit");
            break;
        }
        case 11612: {
            if (strcmp("push", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("push");
            break;
        }
        case 10119: {
            if (strcmp("create", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("create");
            if(create(argv[2])!=0){printf("Error Create project with name %s\nPossible Reasons:\n1. Project %s already existed in sever.\n2. Error communicating with server.\n",argv[2],argv[2]);}
            break;
        }
        case 11600: {
            if (strcmp("destroy", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("destroy");
            break;
        }
        case 10097: {
            if (strcmp("add", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("add");
            break;
        }
        case 11014: {
            if (strcmp("remove", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("remove");
            break;
        }
        case 11499: {
            if (strcmp("currentversion", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("currentversion");
            break;
        }
        case 11604: {
            if (strcmp("history", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("history");
            if(history(argv[2])!=0){printf("Error get project history with name %s\nPossible Reasons:\n1. Project %s does not exist in sever.\n2. Error communicating with server.\n",argv[2],argv[2]);}
            break;
        }
        case 10914: {
            if (strcmp("rollback", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("rollback");
            break;
        }
        default:{
            printf(PARSEERROR);
            return -1;
        }
    }
    //TRACE(("size of a size_t: %d \n", sizeof(size_t)));
    return 0;
}