#include "logic.h"
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

buffer* createProject(parsed_request_t *req){
    int status,fh;
    buffer *response;
    char* path;
    char* proj_name;
    proj_name=(char*)malloc(req->project_name_size+1);
    strncpy(proj_name,req->project_name,req->project_name_size);
    proj_name[req->project_name_size]=0;
    pthread_rwlock_t *rwlock = get_rwlock_for_project(proj_name);
    pthread_rwlock_wrlock(rwlock);
    mkdir("Projects",S_IRUSR|S_IWUSR|S_IXUSR);
    path=(char*)malloc(sizeof(char)*(req->project_name_size+11+9));
    strcat(path,"Projects/");
    strncat(path,req->project_name,req->project_name_size);
    status=mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    if(status==EEXIST){
        response=get_output_buffer_for_response(001,2);
        finalize_buffer(response);
        goto END;
    }
    if(status!=0){
        response=get_output_buffer_for_response(002,2);
        finalize_buffer(response);
        goto END;
    }
    strncat(path,"/.manifest",10);
    fh=writeFile(path,"Made_By_HXX&DZZ\n",16);

    if(fh!=0){
        response=get_output_buffer_for_response(003,2);
        finalize_buffer(response);
        goto END;
    }
    response=get_output_buffer_for_response(000,2);
    appendSequenceBuffer(response,"Made_By_HXX&DZZ\n",16);
    finalize_buffer(response);
    END:
    free_in_packet();
    pthread_rwlock_unlock(rwlock);
    return response;
}

buffer* destroy(parsed_request_t *req){
    int status,fh;
    buffer *response;
    char* path;
    char* proj_name;
    proj_name=(char*)malloc(req->project_name_size+1);
    strncpy(proj_name,req->project_name,req->project_name_size);
    proj_name[req->project_name_size]=0;
    pthread_rwlock_t *rwlock = get_rwlock_for_project(proj_name);
    pthread_rwlock_wrlock(rwlock);
    status=isDir(proj_name);
    if(status!=0){
        response=get_output_buffer_for_response(301,2);// does not exists the project with project name
        finalize_buffer(response);
        goto END;
    }
    asprintf(&path,"rm -rf %s",proj_name);
    system(path);
    response=get_output_buffer_for_response(300,2);// success removing the project
    finalize_buffer(response);
    END:
    free_in_packet();
    pthread_rwlock_unlock(rwlock);
    return response;
}

buffer* history(parsed_request_t *req){

}

buffer* process_logic(parsed_request_t* req) {

    switch(req->op_code){
        case 0 : return createProject(req);
        case 1 : return history(req);
        case 2 : return currentversion(req);
        case 3 : return destroy(req);
        default: return NULL;
    }

}