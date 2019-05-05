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
    char* path,*path_bk=malloc(10000);
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
    strcpy(path_bk,path);
    strcat(path,"/curr");
    status=mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    strncat(path,"/.manifest",10);
    fh=writeFile(path,"Made_By_HXX&DZZ\n",16);
    if(fh!=0){
        response=get_output_buffer_for_response(003,2);
        finalize_buffer(response);
        goto END;
    }
    strcpy(path,path_bk);
    strncat(path,"/0",1);
    status=mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    strncat(path,"/.manifest",10);
    fh=writeFile(path,"Made_By_HXX&DZZ\n",16);
    if(fh!=0){
        response=get_output_buffer_for_response(003,2);
        finalize_buffer(response);
        goto END;
    }
    strcpy(path,path_bk);
    strcat(path,"/currentversion");
    fh=writeFile(path,"0",1);
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
    asprintf(&proj_name,"Projects/%s",proj_name);
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
            exit(0);
        }
        appendSequenceBuffer(output, file_data, file_size);
        free(file_data);
        appendBuffer(output, '\n');
        appendBuffer(output, '\n');
    }

    free_in_packet();

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



/*
 * handler for push request
 *
 * Response:
 *  [900] => successful push
 *  [901] => failed push, reason: invalid manifest sent
 *  [902] => failed push, reason: out of sync, upgrade first
 *
 */
buffer* push(parsed_request_t* req) {
    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
    buffer* output;
    char project_version_path[PATH_MAX];
    char project_path[PATH_MAX];
    char cmd[PATH_MAX + 9];
    char version_buffer[1024];  // should be able to hold maximum integer converted to bytes form LMAO
    int  version_buffer_size;
    const char* cursor;
    char* project_version_path_appender, *project_path_appender;
    int latest_version, uploaded_version;
    TAR* t;

    if (req->files_payload.payload1_size <= 16)
        goto invalid_manifest;

    pthread_rwlock_wrlock(lock);

    latest_version = get_latest_project_version(req->project_name, req->project_name_size);
    cursor = req->files_payload.payload1 + 16;
    sscanf(cursor, "%d", &uploaded_version);
    if (uploaded_version != latest_version + 1) {
        // out of sync error
        goto out_of_sync;
    }

    // prepare path buffer
    get_project_path(project_version_path, req->project_name, req->project_name_size, latest_version + 1);
    project_version_path_appender = project_version_path + strlen(project_version_path);

    get_project_path(project_path, req->project_name, req->project_name_size, -1);
    project_path_appender = project_path + strlen(project_path);


    // create new version folder
    mkdir(project_version_path, 0700);

    // write .Manifest and files.tar
    strcpy(project_version_path_appender, ".Manifest");
    writeFile(project_version_path, req->files_payload.payload1, req->files_payload.payload1_size);

    strcpy(project_version_path_appender, "files.tar");
    writeFile(project_version_path, req->files_payload.payload2, req->files_payload.payload2_size);

    // free input buffer
    free_in_packet();

    // empty out the current version folder
    strcpy(cmd, "rm -rf ");
    strcat(cmd, project_path);
    strcat(cmd, "Curr/*");
    system(cmd);

    // untar our tar file into Curr
    strcpy(project_path_appender, "Curr/");
    tar_open(&t, project_version_path, NULL, O_RDONLY, 0700, TAR_GNU);
    tar_extract_all(t, project_path);
    tar_close(t);

    // copy .Commit from Curr to specific version folder
    strcpy(project_version_path_appender, ".Commit");
    strcpy(project_path_appender, "Curr/.Commit");
    strcpy(cmd, "cp ");
    strcat(cmd, project_path);
    strcat(cmd, " ");
    strcat(cmd, project_version_path);
    system(cmd);

    // copy .Manifest from specific version folder to Curr
    strcpy(project_version_path_appender, ".Manifest");
    strcpy(project_path_appender, "Curr/.Manifest");
    strcpy(cmd, "cp ");
    strcat(cmd, project_version_path);
    strcat(cmd, " ");
    strcat(cmd, project_path);
    system(cmd);

    // change Currentversion file
    version_buffer_size = sprintf(version_buffer, "%d", latest_version + 1);
    strcpy(project_path_appender, ".Currentversion");
    writeFile(project_path, version_buffer, version_buffer_size);

    output = get_output_buffer_for_response(900, 0);
    pthread_rwlock_unlock(lock);
    return output;

invalid_manifest:
    output = get_output_buffer_for_response(901, 0);
    free_in_packet();
    return output;

out_of_sync:
    free_in_packet();
    output = get_output_buffer_for_response(902, 0);
    pthread_rwlock_unlock(lock);
    return output;
}


buffer* process_logic(parsed_request_t* req) {
    if (req->op_code != 0) {
        if (project_exist(req->project_name, req->project_name_size) < 0) {
            // project does not exist
            buffer* output = get_output_buffer_for_response(999, 0);
            finalize_buffer(output);
            return output;
        }

    }
    switch(req->op_code){
        case 0 : return createProject(req);
        case 1 : return history(req);
        case 2 : return currentversion(req);
        case 3 : return destroy(req);
        case 4 : return rollback(req);
        case 5 : return checkout(req);
        case 6 : return update(req);
        case 7 : return upgrade(req);
        case 8 : return commit(req);
        case 9 : return push(req);
        default: return NULL;
    }

}