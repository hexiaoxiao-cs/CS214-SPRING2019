#include "logic.h"
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

buffer* createProject(parsed_request_t *req){
    int status,fh;
    buffer *response;
    char* path;
    pthread_rwlock_t *rwlock = get_rwlock_for_project(req->project_name);
    pthread_rwlock_wrlock(rwlock);
    status=mkdir(req->project_name,DEFFILEMODE);
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
    path=(char*)malloc(sizeof(char)*(req->project_name_size+11));
    strncat(path,req->project_name,req->project_name_size);
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


buffer* process_logic(parsed_request_t* req) {

    switch(req->op_code){
        case 0 : return createProject(req);
        default: return NULL;
    }

}