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
    char path[PATH_MAX],*path_bk=malloc(10000);
    char* proj_name;
//    proj_name=(char*)malloc(req->project_name_size+1);
//    strncpy(proj_name,req->project_name,req->project_name_size);
//    proj_name[req->project_name_size]=0;
//    pthread_rwlock_t *rwlock = get_rwlock_for_project(req->project_name,req->project_name_size);
//    pthread_rwlock_wrlock(rwlock);
    mkdir("Projects",S_IRUSR|S_IWUSR|S_IXUSR);
    get_project_path(path,req->project_name,req->project_name_size,-1);
    //path=(char*)malloc(sizeof(char)*(req->project_name_size+11+9));
    //strcat(path,"Projects/");
    //strncat(path,req->project_name,req->project_name_size);
    status=mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    if(status==EEXIST){
        TRACE(("Received createProject, Error because of existed project\n"));
        response=get_output_buffer_for_response(001,2);
        finalize_buffer(response);
        goto END;
    }
    if(status!=0){
        TRACE(("Received createProject, Error because of mkdir error\n"));
        response=get_output_buffer_for_response(002,2);
        finalize_buffer(response);
        goto END;
    }
    strcpy(path_bk,path);
    strcat(path,"/Curr");
    status=mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    strncat(path,"/.Manifest",10);
    fh=writeFile(path,"Made_By_HXX&DZZ\n0\n",18);
    if(fh!=0){
        TRACE(("Received createProject, Error because of write .Manifest file to Curr folder error\n"));
        response=get_output_buffer_for_response(003,2);
        finalize_buffer(response);
        goto END;
    }
    strcpy(path,path_bk);
    strncat(path,"/0",1);
    status=mkdir(path,S_IRUSR|S_IWUSR|S_IXUSR);
    strncat(path,"/.Manifest",10);
    fh=writeFile(path,"Made_By_HXX&DZZ\n0\n",18);
    if(fh!=0){
        TRACE(("Received createProject, Error because of write .Manifest file to 0 folder error\n"));
        response=get_output_buffer_for_response(003,2);
        finalize_buffer(response);
        goto END;
    }
    strcpy(path,path_bk);
    strcat(path,"/Currentversion");
    fh=writeFile(path,"0",1);
    if(fh!=0){
        TRACE(("Received createProject, Error because of write Currentversion error\n"));
        response=get_output_buffer_for_response(003,2);
        finalize_buffer(response);
        goto END;
    }
    response=get_output_buffer_for_response(000,2);
    appendSequenceBuffer(response,"Made_By_HXX&DZZ\n",16);
    finalize_buffer(response);
    END:
    free_in_packet();
    free(path_bk);
//    pthread_rwlock_unlock(rwlock);
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
//    pthread_rwlock_t *rwlock = get_rwlock_for_project(req->project_name,req->project_name_size);
//    pthread_rwlock_wrlock(rwlock);
    asprintf(&proj_name,"Projects/%s",proj_name);
    status=isDir(proj_name);
    if(status!=0){
        TRACE(("Received Destroy, Error finding the Directory(Maybe DIRNOTEXIST)\n"));
        response=get_output_buffer_for_response(301,2);// does not exists the project with project name
        finalize_buffer(response);
        goto destroy_end;
    }
    asprintf(&path,"rm -rf %s",proj_name);
    system(path);
    response=get_output_buffer_for_response(300,2);// success removing the project
    finalize_buffer(response);
    destroy_end:
    free_in_packet();
//    pthread_rwlock_unlock(rwlock);
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
    int version;
    int i;
    buffer* output;
    char* file_data;
    size_t file_size;
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);

//    pthread_rwlock_rdlock(lock);


    version = get_latest_project_version(req->project_name, req->project_name_size);
    if (version == 0) {
        goto no_history;
    }


    output = get_output_buffer_for_response(100, 0);

    for (i = 1;i <= version;i++) {
        get_project_path(commit_path, req->project_name, req->project_name_size, i);
        strcat(commit_path, ".Commit");
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
//    pthread_rwlock_unlock(lock);
    return output;

no_history:
    free_in_packet();
    TRACE(("Received HISTORY, No History\n"));
    output = get_output_buffer_for_response(101, 0);
    finalize_buffer(output);
//    pthread_rwlock_unlock(lock);
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

//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);

//    pthread_rwlock_rdlock(lock);

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
//    pthread_rwlock_unlock(lock);
    return output;

no_version:
    output = get_output_buffer_for_response(201, 0);
    TRACE(("Received Currentversion, No_Version"));
    finalize_buffer(output);
//    pthread_rwlock_unlock(lock);

    return output;
}

//TODO:DZZ IMPLEMENTATION
//Server: Receive request
//       Validate the Client request by comparing the Project Version Number
//       Save tar file to local new folder and decompress it to the curr folder
//       Change the currentversion file for the project version
//       copy the .Commit file to the corrosponding folder (The place with Tar)
//       Write the .manifest file to the folder and curr



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
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
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

//    pthread_rwlock_wrlock(lock);

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
    strcpy(cmd, "rm -rf ");strcat(cmd, project_path);strcat(cmd, "Curr/*");
    system(cmd);

    // untar our tar file into Curr
    strcpy(project_path_appender, "Curr/");
    tar_open(&t, project_version_path, NULL, O_RDONLY, 0700, TAR_GNU);
    tar_extract_all(t, project_path);
    tar_close(t);

    // copy .Commit from Curr to specific version folder
    strcpy(project_version_path_appender, ".Commit");
    strcpy(project_path_appender, "Curr/.Commit");
    strcpy(cmd, "cp ");strcat(cmd, project_path);strcat(cmd, " ");strcat(cmd, project_version_path);
    system(cmd);

    // copy .Manifest from specific version folder to Curr
    strcpy(project_version_path_appender, ".Manifest");
    strcpy(project_path_appender, "Curr/.Manifest");
    strcpy(cmd, "cp ");strcat(cmd, project_version_path);strcat(cmd, " ");strcat(cmd, project_path);
    system(cmd);

    // change Currentversion file
    version_buffer_size = sprintf(version_buffer, "%d", latest_version + 1);
    strcpy(project_path_appender, "/Currentversion");
    writeFile(project_path, version_buffer, version_buffer_size);

    output = get_output_buffer_for_response(900, 0);
//    pthread_rwlock_unlock(lock);
    return output;

invalid_manifest:
    TRACE(("Received Push, Invalid_manifest Error\n"));
    output = get_output_buffer_for_response(901, 0);
    free_in_packet();
    return output;

out_of_sync:
    TRACE(("Received Push, Out_of_sync Error\n"));
    free_in_packet();
    output = get_output_buffer_for_response(902, 0);
//    pthread_rwlock_unlock(lock);
    return output;
}


//potential error: tar is broken then we lost data!!

buffer* rollback(parsed_request_t* req){
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
    buffer* output;
    char project_version_path[PATH_MAX];
    char project_path[PATH_MAX];
    char cmd[PATH_MAX + 9];
    char version_buffer[1024];  // should be able to hold maximum integer converted to bytes form LMAO
    int  version;
    const char* cursor;
    char* project_version_path_appender, project_path_appender[PATH_MAX],*project_name;
    int latest_version, uploaded_version;
    TAR* t;
    latest_version = get_latest_project_version(req->project_name, req->project_name_size);
    strncpy(version_buffer,req->str_payload.payload,req->str_payload.payload_size);
//    project_name=(char*)malloc(req->str_payload.payload_size+1);
//    strncpy(project_name,req->str_payload.payload,req->str_payload.payload_size);
//
//    version=get_latest_project_version(req->project_name,req->project_name_size);
    sscanf(version_buffer,"%d",&version);

    get_project_path(project_path,req->project_name,req->project_name_size,-1);

//    pthread_rwlock_wrlock(lock);
    if(version>latest_version || version == 0){
        goto rollback_error;
    }
    if(version==latest_version){
        goto rollback_ok; // nothing to do
    }
    if(version<latest_version){
        for(;latest_version>version;latest_version--){
            get_project_path(project_version_path,req->project_name,req->project_name_size,latest_version);
            strcpy(cmd,"rm -rf ");
            strcat(cmd,project_version_path);
            system(cmd);

        }
        strcpy(cmd,"rm -rf ");
        strcat(cmd,project_path);
        strcat(cmd,"/curr");
        system(cmd);
        get_project_path(project_version_path,req->project_name,req->project_name_size,version);
        get_project_path(project_path,req->project_name,req->project_name_size,-1);
        strcpy(project_path_appender,project_path);
        strcpy(cmd,project_path);
        strcat(project_version_path,"/files.tar");
        strcat(cmd,"/Curr");
        tar_open(&t,project_version_path, NULL, O_RDONLY, 0700, TAR_GNU);
        tar_extract_all(t,cmd);
        strcat(project_path,"/Currentversion");
        writeFile(project_path,req->str_payload.payload,req->str_payload.payload_size);
        tar_close(t);
        goto rollback_ok;
    }
    rollback_error:
        TRACE(("Received Rollback, 0 or Received Version Number bigger than current version number\n"));
        output=get_output_buffer_for_response(401,0);
//        pthread_rwlock_unlock(lock);
        free_in_packet();
        return output;
    rollback_ok:
        output=get_output_buffer_for_response(400,0);
//        pthread_rwlock_unlock(lock);
        free_in_packet();
        return output;

}

buffer* checkout(parsed_request_t *req){
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
    buffer* output;
    char project_version_path[PATH_MAX];
    char project_path[PATH_MAX];
    char cmd[PATH_MAX + 9],*tmp;
    int status=0;
    int latest_version;
    size_t size;
//    pthread_rwlock_rdlock(lock);
    latest_version = get_latest_project_version(req->project_name, req->project_name_size);
    if (latest_version == 0)
        goto version_zero;
    get_project_path(project_path,req->project_name,req->project_name_size,latest_version);
    strcat(project_path,"/files.tar");
    status = readFile(project_path,&tmp,&size);
    if(status !=0 ){ goto checkout_error;}
    output=get_output_buffer_for_response(500,1);
    finalize_file_payload1_for_response(output);
    appendSequenceBuffer(output,tmp,size);
    free_in_packet();
    free(tmp);
    finalize_buffer(output);
    return output;
    checkout_error:
        TRACE(("Received Checkout, Error reading 1.tar file\n"));
        output=get_output_buffer_for_response(501,0);
        finalize_buffer(output);
        return output;
version_zero:
        TRACE(("Received Checkout, Checkout version is 0\n"));
    output = get_output_buffer_for_response(502, 0);
finalize_buffer(output);
    return output;
}
//UPDATE logic:
// read current version manifest and send it to the client
buffer* update(parsed_request_t *req){
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
    buffer* output;
    char project_version_path[PATH_MAX];
    char project_path[PATH_MAX];
    char cmd[PATH_MAX + 9],*tmp;
    int status=0;
    size_t size;
//    pthread_rwlock_rdlock(lock);
    get_project_path(project_path,req->project_name,req->project_name_size,-1);
    strcat(project_path,"Curr/.Manifest");
    status = readFile(project_path,&tmp,&size);
    if(status !=0 ){ goto update_error;}
    output=get_output_buffer_for_response(600,0);
    appendSequenceBuffer(output,tmp,size);
    free_in_packet();
    free(tmp);
    finalize_buffer(output);
//    pthread_rwlock_unlock(lock);
    return output;
    update_error:
        TRACE(("Received Update, Error reading Curr/.Manifest file\n"));
        output=get_output_buffer_for_response(601,0);
        finalize_buffer(output);
        free_in_packet();
        free(tmp);
//        pthread_rwlock_unlock(lock);
        return output;

}

buffer* upgrade(parsed_request_t *req){
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
    char *file_data;
    size_t file_size;
    buffer* output;
    char* requested_version_buffer = (char*) malloc(req->str_payload.payload_size + 1);
    int latest_version, requested_version;
    char project_version_path[PATH_MAX];
    char *project_version_path_appender;

    memcpy(requested_version_buffer, req->str_payload.payload, req->str_payload.payload_size);

    requested_version_buffer[req->str_payload.payload_size] = 0;

//    pthread_rwlock_rdlock(lock);

    latest_version = get_latest_project_version(req->project_name, req->project_name_size);
    sscanf(requested_version_buffer, "%d", &requested_version);

    if (requested_version > latest_version || requested_version < 0)
        goto invalid_version;

    if (requested_version == 0)
        goto version_zero;

    // requested version is in range
    get_project_path(project_version_path, req->project_name, req->project_name_size, requested_version);
    project_version_path_appender = project_version_path + strlen(project_version_path);

    free_in_packet();

    output = get_output_buffer_for_response(700, 1);

    // add .Manifest to payload1
    strcpy(project_version_path_appender, ".Manifest");
    if (readFile(project_version_path, &file_data, &file_size) < 0) {
        TRACE(("Possible folder structure corruption, exiting..."));
        exit(0);
    }

    appendSequenceBuffer(output, file_data, file_size);
    free(file_data);

    finalize_file_payload1_for_response(output);


    // add tar file to payload2
    strcpy(project_version_path_appender, "files.tar");
    if (readFile(project_version_path, &file_data, &file_size) < 0) {
        TRACE(("Possible folder structure corruption, exiting..."));
        exit(0);
    }
    appendSequenceBuffer(output, file_data, file_size);
    free(file_data);
    finalize_buffer(output);
//    pthread_rwlock_unlock(lock);
    return output;

version_zero:
    TRACE(("Received Upgrade, Version is 0\n"));
    output = get_output_buffer_for_response(701, 0);
    finalize_buffer(output);
//    pthread_rwlock_unlock(lock);
    return output;

invalid_version:
    TRACE(("Received Upgrade, Invalid_version\n"));
    output = get_output_buffer_for_response(702, 0);
    finalize_buffer(output);
//    pthread_rwlock_unlock(lock);
    return output;
}

buffer* commit(parsed_request_t *req){
//    pthread_rwlock_t* lock = get_rwlock_for_project(req->project_name, req->project_name_size);
    char project_path[PATH_MAX];
    char *file_data;
    size_t file_size;
    buffer* output;

//    pthread_rwlock_rdlock(lock);

    get_project_path(project_path, req->project_name, req->project_name_size, -1);
    strcat(project_path, "Curr/.Manifest");

    free_in_packet();

    if (readFile(project_path, &file_data, &file_size) != 0) {
        TRACE(("Possible folder structure corruption, exiting..."));
        exit(0);
    }


    output = get_output_buffer_for_response(800, 0);
    appendSequenceBuffer(output, file_data, file_size);
    free(file_data);
    finalize_buffer(output);

//    pthread_rwlock_unlock(lock);
    return output;
}



buffer* process_logic(parsed_request_t* req) {
    pthread_rwlock_t *lock;
    buffer *stuff;
    if (req->op_code != 0) {

    }
    lock=get_rwlock_for_project(req->project_name,req->project_name_size);
    switch(req->op_code){
        case 0 :
            pthread_rwlock_wrlock(lock);
            stuff= createProject(req);
            break;
        case 1 :
            pthread_rwlock_rdlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff=output;
            }
            else{ stuff=history(req);}
            break;
        case 2 :
            pthread_rwlock_rdlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }else{
            stuff=currentversion(req);
            }
            break;
        case 3 :
            pthread_rwlock_wrlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }else {
                stuff = destroy(req);
            }
            break;
        case 4 :
            pthread_rwlock_wrlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }
            else{stuff=rollback(req);}
            break;
        case 5 :
            pthread_rwlock_rdlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }
            else{stuff= checkout(req);}
            break;
        case 6 :
            pthread_rwlock_rdlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }else{
            stuff= update(req);}
            break;
        case 7 :
            pthread_rwlock_rdlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }else{
            stuff= upgrade(req);}
            break;
        case 8 :
            pthread_rwlock_rdlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }else
            {stuff= commit(req);}
            break;
        case 9 :
            pthread_rwlock_wrlock(lock);
            if (project_exist(req->project_name, req->project_name_size) < 0) {
                // project does not exist
                buffer* output = get_output_buffer_for_response(999, 0);
                finalize_buffer(output);
                stuff= output;
            }else{
            stuff= push(req);}
            break;
        default: return NULL;
    }
    pthread_rwlock_unlock(lock);
    return stuff;
}

/*
 * TODO:
 */
