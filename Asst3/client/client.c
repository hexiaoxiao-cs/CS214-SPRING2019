#include "client.h"
#include "util.h"
#include <protocol.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define PARSEERROR "Argv Error\nUsage:\n./wtf [configure <IP/hostname> <port>] [checkout <project name>] [update <project name>]\n[upgrade <project name>] [commit <project name>] [push <project name>]\n[create <project name>] [destroy <project name>] [add <project name> <filename>]\n[remove <project name> <filename>] [currentversion <project name>] [history <project name>]\n[rollback <project name> <version>]\n"

char* ipaddr;
int portno;

void writeFile(const char *file_path, char *data, size_t size) {
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
            printf("Error when writing file %s\n",  file_path);
        } else if (ret == 0) {
            break;
        } else {
            //Positive interger
            tmp += ret;
        }
    }
    close(handler);    //Close file
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

int create(char* project_name){
    int op=0;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    //send packet
    //receive packet
    //read .manifest
    parse_response(NULL);

    return 0;
}
int history(char* project_name){
    int op =1;
    char *the_history;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    //send packet
    //receive packet
    parse_response(NULL);
    printf("%s",the_history);
    return 0;

}

int current_version(char* project_name){
    int op=2;
    get_output_buffer_for_request(op,project_name,strlen(project_name));


    return 0;
}
int destroy(char* project_name){
    int op=3;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
}
int rollback(char* project_name){
    int op=4;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int checkout(char* project_name){
    int op=5;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int update(char* project_name){
    int op=6;
    get_output_buffer_for_request(op,project_name,strlen(project_name));

    return 0;
}
int upgrade(char* project_name){
    int op=7;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int commit(char* project_name){
    int op=8;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
    return 0;
}
int push(char* project_name){
    int op = 9 ;
    get_output_buffer_for_request(op,project_name,strlen(project_name));
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

    if((argv[1][0]+argv[1][2]*100)!=11099) {
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