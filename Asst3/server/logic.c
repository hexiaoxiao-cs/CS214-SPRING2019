#include "logic.h"
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <serverutil.h>

#include <limits.h>

// TODO: create Project resee

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

//Folder Structure
//-|
// |---WTFserver
// |---Projects
//     |---Project_Name_1
//     | ......
//     |---Project_Name_N
//         |---0
//         |---1 (->Project Version Number)
//         | ....
//         |---n (->Current Version Number)
//             |---files.tar
//             |---.Commit
//             |---.Manifest
//         |---Currentversion (file store a number)
//         |---Curr (folder store the current files)
//             |---.Manifest
//             |---.....


/*
 * handler for history request
 *
 * Response:
 *  [100] => successful query, latest displayable history will be in str_payload
 *  [101] => failed query, reason: no history information available (current version is zero)
 *
 */
buffer* history(parsed_request_t *req){
    char commit_path[PATH_MAX];
    int version = get_latest_project_version(req->project_name, req->project_name_size);
    int i;
    buffer* output;
    char* file_data;
    size_t file_size;

    if (version == 0) {
        goto no_history;
    }

    output = get_output_buffer_for_response(100, 0);

    for (i = 1;i <= version;i++) {
        get_project_path(commit_path, req->project_name, req->project_name_size, i);
        if (readFile(commit_path, &file_data, &file_size) < 0) {
            TRACE(("Possible directory structure corruption, exiting... \n"));
            exit(0)
        }
        appendSequenceBuffer(output, file_data, file_size);
        free(file_data);
        appendBuffer(output, '\n');
        appendBuffer(output, '\n');
    }

    finalize_buffer(output);
    return output;

no_history:
    output = get_output_buffer_for_response(101, 0);
    finalize_buffer(output);
    return output;
}


/*
 * handler for currentversion request
 *
 * Response:
 *  [200] => successful query, latest manifest will be in str_payload
 *  [201] => failed query, reason: no version information available (current version is zero)
 *
 */
buffer* currentversion(parsed_request_t* req) {
    char manifest_path[PATH_MAX];
    char *file_data;
    size_t file_size;
    int project_version;
    buffer* output;

    project_version = get_latest_project_version(req->project_name, req->project_name_size);
    get_project_path(manifest_path, req->project_name, req->project_name_size, project_version);

    free_in_packet();

    if (project_version == 0) {
        goto no_version;
    }

    strcat(manifest_path, ".Manifest"); // Projects/${project_name}/${project_version}/.Manifest
    if (readFile(manifest_path, &file_data, &file_size) < 0) {
        TRACE(("Possible directory structure corruption, exiting... \n"));
        exit(0);
    }

    output = get_output_buffer_for_response(200, 0);
    appendSequenceBuffer(output, file_data, file_size);
    free(file_data);
    finalize_buffer(output);

    return output;

no_version:
    output = get_output_buffer_for_response(201, 0);
    finalize_buffer(output);
    return output;
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