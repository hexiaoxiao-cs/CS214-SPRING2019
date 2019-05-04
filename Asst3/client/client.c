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
#include <libtar.h>
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
//return -1 indicating make_new_manifest deleted something
int make_new_manifest(manifest_item **array,int *counts,char*** deleted_files,size_t *deleted_counts){
    int temp=0;
    char* file_data;
    int status,isDeleted=0;
    size_t size,deleted;
    deleted=0;
    char sha256buf[65];
    (*deleted_files)=(char**) malloc(sizeof(char*));
    for(temp=0;temp<*counts;temp++){
        status=readFile(array[temp]->filename->data,&file_data,&size);
        array[temp]->newhash=createBuffer();

        //if not found, we will destroy the manifest item
        if(status!=0){
            (*deleted_files)=realloc((*deleted_files), sizeof(char*)*(deleted+1));
            (*deleted_files)[deleted]=array[temp]->filename->data;
            deleted++;
            if(temp!=*counts-1){
            free(array[temp]);
            array[temp]=array[*counts-1];
                (*counts)--;
        }
            else{
                free(array[temp]);
                (*counts)--;
            }
            isDeleted=1;
            continue;}// -1 means not found (or deleted)
        sha256_string(file_data,size,sha256buf);
        appendSequenceBuffer(array[temp]->newhash,sha256buf,65);
    }
    *deleted_counts=deleted;
    if(isDeleted==1){return -1;}
    return 0;
}

int create(char* project_name){
    int op=0;
    buffer *output,*input;
    parsed_response_t response;
    output=get_output_buffer_for_request(op,project_name,strlen(project_name),0);
    send_request(ipaddr,portno,output,&input);
    parse_response(input,&response);
    if(strcmp(response.str_payload.payload,"OK")!=0){return 0;}
    else{return -1;}
}
int history(char* project_name){
    int op =1;
    char *the_history;
    buffer *output,*input;
    parsed_response_t response;
    output=get_output_buffer_for_request(op,project_name,strlen(project_name),0);
    send_request(ipaddr,portno,output,&input);
    parse_response(input,&response);
    if(response.str_payload.payload_size!=0){return -1;}
    printf("%s",response.str_payload.payload);
    return 0;
}

//int current_version(char* project_name){
//    int op=2;
//    buffer *output,*input;
//    get_output_buffer_for_request(op,project_name,strlen(project_name));
//
//
//    return 0;
//}

int destroy(char* project_name){
    int op=3;
    buffer *output,*input;
    parsed_response_t response;
    output=get_output_buffer_for_request(op,project_name,strlen(project_name),0);
    send_request(ipaddr,portno,output,&input);
    parse_response(input,&response);
    if(strcmp(response.str_payload.payload,"OK")!=0){return 0;}
    else{return -1;}

}
//int rollback(char* project_name){
//    int op=4;
//    buffer *output,*input;
//    get_output_buffer_for_request(op,project_name,strlen(project_name));
//    return 0;
//}
//int checkout(char* project_name){
//    int op=5;
//    buffer *output,*input;
//    get_output_buffer_for_request(op,project_name,strlen(project_name));
//    return 0;
//}
//int update(char* project_name){
//    int op=6;
//    buffer *output,*input;
//    get_output_buffer_for_request(op,project_name,strlen(project_name));
//
//    return 0;
//}
//int upgrade(char* project_name){
//    int op=7;
//    buffer *output,*input;
//    get_output_buffer_for_request(op,project_name,strlen(project_name));
//    return 0;
//}
//server
int commit(char* project_name){
    int op=8;
    buffer *output,*input;
    parsed_response_t response;
    get_output_buffer_for_request(op,project_name,strlen(project_name),1);
    send_request(ipaddr,portno,output,&input);
    parse_response(input,&response);
    if(response.status_code!=800){return -1;}
    return 0;
}

//Push Work Flow:
//Local: Check if all files in the manifest has been changed or changes recorded in the .Commit File
//       Increment the Project Number and File Version Number
//       Send to the Server the tar file and the newly written Manifest (remember to tar the .Commit file
//       Wait for server to give OK respond
//       Write Manifest to the local storage and delete .Commit File


//Return Code:
//O -> OK
//-1 ->Manifest Read Error
//-2 ->Manifest Corrupted
//-3 ->.Commit Read Error
//-4 ->.Commit Corrupted
int push(char* project_name){
    int op = 9;
    buffer *output, *input;
    char *manifest_path, *commit_path, **deleted_files, *actual_path,*inside_path;
    char *file_info,*stuff,*tar_info;
    size_t size = 0, counts = 0, deleted_counts = 0, new_size = 0, t1 = 0, t2 = 0,tar_size=0;
    int status, temp = 0;
    project my_project;
    TAR *tar;
    manifest_item **Changelog, **Generate;
    parsed_response_t out;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 1);//two payload
    asprintf(&manifest_path, "%s/.manifest", project_name);
    asprintf(&commit_path, "%s/.Commit", project_name);
    status = readFile(manifest_path, &file_info, &size);
    if (status != 0) { return -1; }
    status = readManifest(file_info, size, &my_project);
    if (status != 0) { return -2; }
    status = readFile(commit_path, &file_info, &size);
    if (status != 0) { return -3; }
    readChangeLogFile(&Changelog, &file_info, size, &counts);
    if (make_new_manifest(my_project.manifestItem, &(my_project.many_Items), &deleted_files, &deleted_counts) == -1) {
        printf("Error: Please commit the following deleted files\n");
        for (temp = 0; temp < deleted_counts; temp++) {
            printf("%s\n", deleted_files[temp]);
        }
        writeManifest(&file_info, &my_project, 0);
        writeFile(manifest_path, file_info, strlen(file_info));
        return -4;
    }
    compareManifest(0, my_project.manifestItem, NULL, &Generate, NULL, my_project.many_Items, 1, 0, 1, &new_size, NULL);
    if (new_size != counts) {
        printf("Error: There are uncommitted changes!\nPlease Commit then Push!\n");
    }
    //TODO:Implement check whether two lists are identical (HASH)

    proecessManifest_ByChangelist_Push(&my_project, Changelog, counts);
    writeManifest(&file_info,&my_project,0);
    asprintf(&stuff, "%s/.tmp.manifest", project_name);
    writeFile(stuff,file_info,strlen(file_info));
    //start to tar
    tar_open(&tar, "tmp.tar", NULL, O_WRONLY | O_CREAT, 0700, TAR_GNU);
    for(temp=0;temp<my_project.many_Items;temp++){
        asprintf(&actual_path,"%s/%s",project_name,my_project.manifestItem[temp]->filename->data);
        tar_append_file(tar,actual_path,my_project.manifestItem[temp]->filename->data);
    }
    asprintf(&inside_path,".tmp.manifest");
    tar_append_file(tar,stuff,inside_path);
    asprintf(&actual_path,"%s/.Commit",project_name);
    asprintf(&inside_path,".Commit");
    tar_append_file(tar,actual_path,inside_path);
    tar_close(tar);
    appendSequenceBuffer(output,file_info,strlen(file_info));
    finalize_file_payload1_for_request(output);
    readFile("tmp.tar",&tar_info,&tar_size);
    appendSequenceBuffer(output,tar_info,tar_size);
    finalize_buffer(output);
    if(send_request(ipaddr,portno,output,&input)==1){return -5;}
    parse_response(input,&out);
    if(out.status_code==900){return 0;}
    else{return -9;}
    //return 0;
}

//0 -> OK!
//-1-> Manifest Reading Error Potential Error: Project Name Does Not Exists
//-2-> Path Not Correct Format
int add(char* project_name, char* to_add_path){
    char* manifest_path;
    size_t size;
    project curr;
    char *regulized_path;
    int status=0;
    asprintf(&manifest_path,"%s/.manifest",project_name);
    char* manifest_raw;
    manifest_item *new ;
    status=readFile(manifest_path,&manifest_raw,&size);
    if(status!=0){return -1;} // -1 -> Manifest Reading Error
    readManifest(manifest_raw,size,&curr);
    curr.manifestItem=(manifest_item**)realloc(curr.manifestItem,sizeof(manifest_item*)*(curr.many_Items+1));
    asprintf(&regulized_path,"%s/%s",project_name,to_add_path);
    regulized_path=is_valid_path(regulized_path);
    if(regulized_path==NULL){return -2;}
    status=open(regulized_path,O_RDONLY);
    if(status!=0){return -2;}
    new= (manifest_item*) malloc(sizeof(manifest_item));
    new->version_num=0;
    new->hash=createBuffer();
    appendSequenceBuffer(new->hash,"0",1);
    new->filename=createBuffer();
    appendSequenceBuffer(new->filename,to_add_path,strlen(to_add_path));
    new->filename_64 =createBuffer();
    size=0;
    appendSequenceBuffer(new->filename_64,base64_encode(to_add_path,strlen(to_add_path),&size),size);
    curr.manifestItem[curr.many_Items]=new;
    curr.many_Items++;
    sort_manifest(curr.manifestItem,curr.many_Items);
    writeManifest(&manifest_raw,&curr,0);
    return writeFile(manifest_path,manifest_raw,strlen(manifest_raw));

}

int remove_entry(char* project_name, char* path){
    char* manifest_path;
    size_t size,temp;
    project curr;
    int status=0;
    asprintf(&manifest_path,"%s/.manifest",project_name);
    char* manifest_raw;
    char* base=base64_encode(path,strlen(path),&size);
    //manifest_item *new = (manifest_item*) malloc(sizeof(manifest_item));
    status=readFile(manifest_path,&manifest_raw,&size);
    if(status!=0){return -1;} // -1 -> Manifest Reading Error
    readManifest(manifest_raw,size,&curr);
    for(temp=0;temp<curr.many_Items;temp++){
        if(strcmp(curr.manifestItem[temp]->filename_64->data,base)==0){break;}
    }
    if(temp==curr.many_Items-1){
        curr.many_Items--;
    }
    else {
        curr.manifestItem[temp]=curr.manifestItem[curr.many_Items-1];
        curr.many_Items--;
        sort_manifest(curr.manifestItem,curr.many_Items);
    }
    writeManifest(&manifest_raw,&curr,0);
    return writeFile(manifest_path,manifest_raw,strlen(manifest_raw));
    return 0;
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
        if (readConfigure() == -1) { printf("Error Happened During Reading Configure File\n"); return -1; }
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
            //if(checkout(argv[3])!=0){printf("Error checkout project with name %s\nPossible Reasons:\n1. Project %s does not exist in sever.\n2. Error communicating with server.\n",argv[2],argv[2]);}
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
            if(add(argv[2],argv[3])!=0){printf("Error Add file %s into Project %s\nPossible Reason:\n1. Project %s not existed locally.\n2. File not exist.\n",argv[3],argv[2],argv[2]);}
            //printf("add");
            break;
        }
        case 11014: {
            if (strcmp("remove", argv[1]) != 0) {
                printf(PARSEERROR);
                return -1;
            }
            remove_entry(argv[2],argv[3]);
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