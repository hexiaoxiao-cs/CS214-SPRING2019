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
#include <limits.h>

#define PARSEERROR "Argv Error\nUsage:\n./wtf [configure <IP/hostname> <port>] [checkout <project name>] [update <project name>]\n[upgrade <project name>] [commit <project name>] [push <project name>]\n[create <project name>] [destroy <project name>] [add <project name> <filename>]\n[remove <project name> <filename>] [currentversion <project name>] [history <project name>]\n[rollback <project name> <version>]\n"

char *ipaddr;
int portno;

int status;

//using openSSL hashing library

void sha256_string(const char *data, size_t len, char outputBuffer[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int i;
    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(hash, &sha256);

    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

int sha256_file(char *path, char outputBuffer[65]) {
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
int make_new_manifest(const char* project_name, manifest_item **array, int *counts, char ***deleted_files, size_t *deleted_counts) {
    int temp = 0;
    char *file_data;
    int isDeleted = 0;
    size_t size, deleted;
    deleted = 0;
    char sha256buf[65];
    char file_path[PATH_MAX];
    char *file_path_appender;

    strcpy(file_path, project_name);
    strcat(file_path, "/");
    file_path_appender = file_path + strlen(file_path);

    (*deleted_files) = (char **) malloc(sizeof(char *));
    for (temp = 0; temp < *counts; temp++) {
        strcpy(file_path_appender, array[temp]->filename->data);
        status = readFile(file_path, &file_data, &size);
        array[temp]->newhash = createBuffer();

        //if not found, we will destroy the manifest item
        if (status != 0) {
            (*deleted_files) = realloc((*deleted_files), sizeof(char *) * (deleted + 1));
            (*deleted_files)[deleted] = array[temp]->filename->data;
            deleted++;
            if (temp != *counts - 1) {
                free(array[temp]);
                array[temp] = array[*counts - 1];
                temp--;
                (*counts)--;
            } else {
                free(array[temp]);
                (*counts)--;
            }
            isDeleted = 1;
            continue;
        }// -1 means not found (or deleted)
        sha256_string(file_data, size, sha256buf);
        appendSequenceBuffer(array[temp]->newhash, sha256buf, 65);
    }
    *deleted_counts = deleted;
    sort_manifest(array,*counts);
    if (isDeleted == 1) { return -1; }
    return 0;
}

int handle_error(parsed_response_t *res) {
    if (res->status_code % 100 != 0) {
        printf("Response not success, reason: ");
        switch (res->status_code) {
            case 999:
                printf("Such project does not exist in the server");
                break;
            case 001:
                printf("An project named that already existed");
                break;
            case 002:
                printf("An mkdir error occured during createProject");
                break;
            case 003:
                printf("Could not perform disk operation during createProject");
                break;
            case 301:
                printf("Project does not exist");   // this should never happen
                break;
            case 101:
                printf("No history information available");
                break;
            case 201:
                printf("No version information available [maybe this is a new project?]");
                break;
            case 901:
                printf("Invalid manifest received by the server");
                break;
            case 902:
                printf("Client is out of sync, please update/upgrade first");
                break;
            case 401:
                printf("Received invalid version number");
                break;
            case 501:
                printf("Unable to read tarfile in the server");
                break;
            case 502:
                printf("No version is available from the server");
                break;
            case 601:
                printf("Error reading Curr/.Manifest file");
                break;
            case 701:
                printf("Version number 0 received in Upgrade");
                break;
            case 702:
                printf("Invalid version number received");
                break;
        }
        printf("\n");
        return 1;
    }
    return 0;
}

int create(char *project_name) {
    int op = 0;
    buffer *output, *input;
    parsed_response_t response;
    char* path,*contents;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    finalize_buffer(output);
    status = send_request(ipaddr, portno, output, &input);
    if (status != 0) {
        printf("Error sending request\n");
        return -2;
    }
    if (parse_response(input, &response) < 0) {
        printf("Error Parsing request\n");
        return -3;
    }
    if (handle_error(&response))
        return -4;
    if (response.status_code == 000) {
        asprintf(&path,"%s",project_name);
        mkdir(path,0700);
        asprintf(&path,"%s/.Manifest",project_name);
        asprintf(&contents,"Made_By_HXX&DZZ\n0\n");
        writeFile(path,contents,18);
        return 0; }

}


int history(char *project_name) {
    int op = 1;
    char *the_history;
    buffer *output, *input;
    parsed_response_t response;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    finalize_buffer(output);
    status = send_request(ipaddr, portno, output, &input);
    if (status != 0)
        return -2;
    if (parse_response(input, &response) < 0)
        return -3;
    if (handle_error(&response))
        return -4;
    printf("Status Code: 1->U, 2->M, 3->A, 4->D\n");
    printf("Status Code|File Name|File Name_Base64|File Version|Old Hash|New Hash\n");
    printf("%.*s", response.str_payload.payload_size, response.str_payload.payload);
    return 0;
}

int current_version(char *project_name) {
    int op = 2;
    int status, read = 0;
    buffer *output, *input;
    parsed_response_t response;
    size_t curr = 0,cnm=0;
    long version;
    const char *res;
    char* caonimabi;
    char*file_64 = malloc(100000);
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    finalize_buffer(output);
    status = send_request(ipaddr, portno, output, &input);
    if (status != 0)
        return -2;
    if (parse_response(input, &response) < 0)
        return -3;
    if (handle_error(&response))
        return -4;
    res = response.str_payload.payload;
    res += 16;
    sscanf(res, "%ld\n%n", &version, &read);
    printf("Project Version: %ld\n", version);
    printf("File Name: \tFile Version:\t\n");
    res += read;
    curr += read;
    read = 0;
    while (curr < response.str_payload.payload_size - 16) {
        sscanf(res, "%s %ld %*s\n%n", file_64, &version, &read);
        caonimabi=base64_decode(file_64, strlen(file_64), &cnm);
        printf("%.*s %ld\n",cnm,caonimabi , version);
        res += read;
        curr += read;
        read = 0;
    }
    return 0;
}

int destroy(char *project_name) {
    int op = 3, status = 0;
    buffer *output, *input;
    parsed_response_t response;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    status = send_request(ipaddr, portno, output, &input);
    if (status != 0)
        return -2;
    if (parse_response(input, &response) < 0)
        return -3;
    if (handle_error(&response))
        return -4;
    return 0;

}

int rollback(char *project_name, char* version) {
    int op = 4, status = 0;
    buffer *output, *input;
    parsed_response_t response;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    appendSequenceBuffer(output,version,strlen(version));
    finalize_buffer(output);
    status = send_request(ipaddr, portno, output, &input);
    if (status != 0){
        printf("Error Send Request\n");
        return -2;

    }
    if (parse_response(input, &response) < 0) {
        printf("Error Parse Response\n");
        return -3;
    }
    if (handle_error(&response))
        return -4;
    return 0;
}

int checkout(char *project_name) {
    int op = 5, status = 0;
    buffer *output, *input;
    char a, *temp;
    TAR* t;
    parsed_response_t response;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    printf("WARNING!\nThis operation will overwrite all necessary files in the project folder to \"reset\" the repository to the current server version.\nProceed?(Y\\N)\n");
    scanf("%c", &a);
    if (a == 'N' || a == 'n') { return -1; }
    finalize_buffer(output);
    status = send_request(ipaddr, portno, output, &input);
    if (status != 0) {
        printf("Error Sending Request\n");
        return -2;
    }
    if (parse_response(input, &response) < 0) {
        printf("Error Parse Request\n");
        return -3;
    }
    if (handle_error(&response))
        return -4;
    writeFile("tmp.tar", response.files_payload.payload2, response.files_payload.payload2_size);
    tar_open(&t, "tmp.tar", NULL, O_RDONLY, 0700, TAR_GNU);
    asprintf(&temp, "%s/", project_name);
    tar_extract_all(t, temp);
    tar_close(t);
    return 0;
}




int update(char *project_name) {
    int op = 6, nstatus = 0;
    buffer *output, *input;
    parsed_response_t response;
    project server, client;
    char *client_manifest_path, *client_manifest_file, *update_file,**deleted_files;
    size_t size_1 = 0, size_2 = 0,deleted_counts;
    int count_1 = 0, count2 = 0, status = 0;
    manifest_item **changelist, **conflicts;
    asprintf(&client_manifest_path, "%s/.Manifest", project_name);
    status = readFile(client_manifest_path, &client_manifest_file, &size_1);
    if (status != 0) { return -1; }
    status = readManifest(client_manifest_file, size_1, &client);
    if (status != 0) { return -1; }
    make_new_manifest(project_name,client.manifestItem,&(client.many_Items),&deleted_files,&deleted_counts);
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    finalize_buffer(output);
    nstatus = send_request(ipaddr, portno, output, &input);
    if (nstatus != 0)
        return -2;
    if (parse_response(input, &response) < 0)
        return -3;
    if (handle_error(&response))
        return -4;
    status = readManifest(response.str_payload.payload, response.str_payload.payload_size, &server);
    if (status != 0) { return -1; }
    status = compareManifest(0, client.manifestItem, server.manifestItem, &changelist, &conflicts, client.many_Items,
                             server.many_Items, client.project_version, server.project_version, &size_1, &size_2, 1);


    if (status == -9) {
        printf("Conflicts occurred!\n");
        for (count_1 = 0; count_1 < size_2; count_1++) {
            printf("%s\n", conflicts[count_1]->filename->data);
        }
        return -1;
    } else {
        for(count2=0;count2<size_1;count2++){
            if(changelist[count2]->changecode==1){
                //        conflicts
                printf("Conflicts: %s\n",changelist[count2]->filename->data);

            }
        }
        writeChangeLogFile(changelist, &update_file, size_1, 1, server.project_version);
        asprintf(&client_manifest_path, "%s/.Update", project_name);
        writeFile(client_manifest_path, update_file, strlen(update_file));
    }
    return 0;
}


int upgrade(char *project_name) {
    int op = 7, nstatus = 0;
    int counts = 0, tmp = 0, conflicts = 0;
    buffer *output, *input;
    TAR *t;
    char *changelog_char, *changelog_path, *temp_str, *tmp_path, sha256[65], *output_path;
    manifest_item **changelog;
    size_t changelog_size, str_size;
    long version;
    project manifest, server;
    parsed_response_t response;
    asprintf(&changelog_path, "%s/.Update", project_name);
    status = readFile(changelog_path, &changelog_char, &changelog_size);
    if (status != 0) { printf("Read .Update Error\n");return -1; }
    status = readChangeLogFile(&changelog, &changelog_char, changelog_size, &counts, &version);
    if (status == -1) { printf("Parse .Update Error\n");goto Error_Processing; }
    if(counts==0){printf("Nothing to Update!\n");goto Error_Processing;}
    asprintf(&changelog_path, "%s/.Manifest", project_name);
    status = readFile(changelog_path, &changelog_char, &changelog_size);
    if (status != 0) { printf("Read .Manifest Error\n");goto Error_Processing; }
    status = readManifest(changelog_char, changelog_size, &manifest);
    if (status == -1) { printf("Parse .Manifest Error\n");goto Error_Processing; }
    //make_new_manifest(changelog,&counts,&deleted_files,&deleted_counts);
    for (tmp = 0; tmp < counts; tmp++) {
        if (changelog[tmp]->changecode == 2) {
            asprintf(&tmp_path, "%s/%s", project_name, changelog[tmp]->filename->data);
            status = readFile(tmp_path, &temp_str, &str_size);
            if (status == 0) {
                sha256_string(temp_str, strlen(temp_str), sha256);
                if (strcmp(sha256, changelog[tmp]->newhash->data) != 0) {
                    if (conflicts == 0) {
                        printf("Conflicts Found:\n");
                        conflicts = 1;
                    }
                    printf("%s\n", changelog[tmp]->filename->data);
                }

            }
        }
    }
    if (conflicts == 1) { goto Error_Processing; }
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    asprintf(&temp_str, "%d", (int)version);
    appendSequenceBuffer(output, temp_str, strlen(temp_str));
    finalize_buffer(output);
    nstatus = send_request(ipaddr, portno, output, &input);
    if (nstatus != 0) {
        printf("Error Sending Request\n");
        return -2;
    }
    status = parse_response(input, &response);
    if (status != 0) { printf("Error Parsing Respond\n");return -3; }

    if (handle_error(&response))
        goto Error_Processing;

    writeFile("tmp.tar", response.files_payload.payload2, response.files_payload.payload2_size);
    //tar_open(&t,"tmp.tar",NULL, O_RDONLY | O_CREAT, 0700, TAR_GNU);
    asprintf(&output_path, "%s/", project_name);
    for (tmp = 0; tmp < counts; tmp++) {
        tar_extract_specific_file("tmp.tar", changelog[tmp]->filename->data, output_path);
    }
    readManifest(response.files_payload.payload1, response.files_payload.payload1_size, &server);
    proecessManifest_ByChangelist_Update(&manifest, changelog, counts, &server);
    writeManifest(&temp_str, &server, 0);
    writeFile(changelog_path, temp_str, strlen(temp_str));
    asprintf(&changelog_path,"rm %s/.Update",project_name);
    system(changelog_path);
    return 0;
    Error_Processing:
        asprintf(&changelog_path,"rm %s/.Update",project_name);
        system(changelog_path);
        return -1;
}

//server

int commit(char *project_name) {
    int op = 8;
    buffer *output, *input;
    parsed_response_t response;
    project server, client;
    manifest_item **changelog, **conflicts;
    size_t changelog_size = 0, conflict_size = 0;
    int status = 0, nstatus = 0, temp;
    size_t manifest_size, deleted_size = 0;
    char *manifest_path, *commit_path,*server_path, *manifest, **deleted_files, *changelog_char;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 0);
    finalize_buffer(output);
    nstatus = send_request(ipaddr, portno, output, &input);
    if (nstatus != 0) {
        printf("Error Sending Request\n");
        return -2;
    }
    if (parse_response(input, &response) < 0) {
        printf("Error Parse Response\n");
        return -3;
    }
    if (handle_error(&response))
        return -4;
    if (readManifest(response.str_payload.payload, response.str_payload.payload_size, &server) != 0) { printf("Error Reading Server .Manifest\n");return -1; }
    asprintf(&manifest_path, "%s/.Manifest", project_name);
    asprintf(&commit_path, "%s/.Commit", project_name);
    asprintf(&server_path, "%s/.Server", project_name);
    writeFile(server_path,response.str_payload.payload,response.str_payload.payload_size);
    readFile(manifest_path, &manifest, &manifest_size);
    readManifest(manifest, manifest_size, &client);
    status = make_new_manifest(project_name, client.manifestItem, &(client.many_Items), &deleted_files, &deleted_size);
    if (status == -1) {
        printf("Warning: Following files are deleted.\n");
        for (temp = 0; temp < deleted_size; temp++) {
            printf("%s\n", deleted_files[temp]);
        }
        writeManifest(&changelog_char,&client,0);
    }
    status = compareManifest(0, client.manifestItem, server.manifestItem, &changelog, &conflicts, client.many_Items,
                             server.many_Items, client.project_version, server.project_version, &changelog_size,
                             &conflict_size, 1);
    if (status == -9) {
        printf("Conflicts Detected\nFollowing files are conflicts.\n");
        for (temp = 0; temp < conflict_size; temp++) {
            printf("%s\n", conflicts[temp]->filename->data);
        }
        printf("Suggest Actions: 1.Make backup of the files below.\n2. Execute checkout to rewrite local files.\n");
        return -1;
    }
    writeChangeLogFile(changelog, &changelog_char, changelog_size, 2, server.project_version);
    writeFile(commit_path, changelog_char, strlen(changelog_char));
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
int push(char *project_name) {
    int op = 9, counts = 0;
    buffer *output, *input;
    char *manifest_path, *commit_path,*server_path, **deleted_files, *actual_path, *inside_path;
    char *file_info, *stuff, *tar_info;
    size_t size = 0, deleted_counts = 0, new_size = 0, t1 = 0, t2 = 0, tar_size = 0,conflict_size=0;
    int status, temp = 0,cnm=0;
    long rubbish = 0;
    project my_project,server;
    TAR *tar;
    manifest_item **Changelog, **Generate,**conflicts;
    parsed_response_t out;
    output = get_output_buffer_for_request(op, project_name, strlen(project_name), 1);//two payload
    asprintf(&manifest_path, "%s/.Manifest", project_name);
    asprintf(&commit_path, "%s/.Commit", project_name);
    asprintf(&server_path, "%s/.Server", project_name);
    status = readFile(manifest_path, &file_info, &size);
    if (status != 0) { printf("Error Reading .Manifest\n");return -1; }
    status = readManifest(file_info, size, &my_project);
    if (status != 0) { printf("Error Parsing .Manifest\n");return -2; }
    status = readFile(server_path,&file_info,&size);
    if (status != 0) { printf("Error Reading Previous cached server's .Manifest\n");return -2; }
    status=readManifest(file_info,size,&server);
    if (status != 0) { printf("Error Parsing .Manifest\n");return -2; }
    status = readFile(commit_path, &file_info, &size);
    if (status != 0) { printf("Error Reading .Commit\n");return -3; }
    status=readChangeLogFile(&Changelog, &file_info, size, &counts, &rubbish);
    if(status!=0){printf("Error Parsing .Commit\n"); return -4;}
    if (make_new_manifest(project_name, my_project.manifestItem, &(my_project.many_Items), &deleted_files, &deleted_counts) == -1) {
        printf("Error: Please commit the following deleted files\n");
        for (temp = 0; temp < deleted_counts; temp++) {
            printf("%s\n", deleted_files[temp]);
        }
        writeManifest(&file_info, &my_project, 0);
        writeFile(manifest_path, file_info, strlen(file_info));
        return -4;
    }
    compareManifest(0, my_project.manifestItem, server.manifestItem, &Generate, &conflicts, my_project.many_Items, server.many_Items,my_project.project_version, server.project_version, &new_size,
                    &conflict_size, 1);
    if (new_size != counts) {
        printf("Error: There are uncommitted changes!\nPlease Commit then Push!\n");
        return -1;
    }
    proecessManifest_ByChangelist_Push(&my_project, Changelog, counts);
    if(counts==0){
        printf("There nothing to commit.\n");
        asprintf(&inside_path,"rm %s/.Commit",project_name);
        system(inside_path);
        return -1;
    }
    writeManifest(&file_info, &my_project, 0);
    asprintf(&stuff, "%s/.tmp.Manifest", project_name);
    writeFile(stuff, file_info, strlen(file_info));
    //start to tar
    tar_open(&tar, "tmp.tar", NULL, O_WRONLY | O_CREAT | O_TRUNC, 0700, TAR_GNU);
    for (temp = 0; temp < my_project.many_Items; temp++) {
        asprintf(&actual_path, "%s/%s", project_name, my_project.manifestItem[temp]->filename->data);
        tar_append_file(tar, actual_path, my_project.manifestItem[temp]->filename->data);
    }
    asprintf(&inside_path, ".Manifest");
    tar_append_file(tar, stuff, inside_path);
    asprintf(&actual_path, "%s/.Commit", project_name);
    asprintf(&inside_path, ".Commit");
    tar_append_file(tar, actual_path, inside_path);
    tar_close(tar);
    appendSequenceBuffer(output, file_info, strlen(file_info));
    finalize_file_payload1_for_request(output);
    fastReadFile("tmp.tar", output);
    finalize_buffer(output);
    if (send_request(ipaddr, portno, output, &input) == 1) { printf("Error Send Request\n");return -5; }
    if (parse_response(input, &out) < 0) {
        asprintf(&inside_path,"rm %s/.Commit",project_name);
        system(inside_path);
        printf("Error Parse Response\n");
        return -3;
    }
    if (handle_error(&out))
        return -9;
    if (out.status_code == 900) {
        asprintf(&stuff,"%s/.Manifest",project_name);
        writeFile(stuff,file_info,strlen(file_info));
        asprintf(&inside_path,"rm %s/.Commit",project_name);
        system(inside_path);
        return 0;
    }
    //return 0;
}

//0 -> OK!
//-1-> Manifest Reading Error Potential Error: Project Name Does Not Exists
//-2-> Path Not Correct Format
int add(char *project_name, char *to_add_path) {
    char *manifest_path;
    size_t size;
    project curr;
    char *regulized_path;
    char open_path[PATH_MAX];
    int status = 0;
    asprintf(&manifest_path, "%s/.Manifest", project_name);
    char *manifest_raw;
    char *base64_encoded;
    int tmp=0;
    manifest_item *new;
    status = readFile(manifest_path, &manifest_raw, &size);
    if (status != 0) { printf("Error Reading .Manifest file\n");return -1; } // -1 -> Manifest Reading Error
    readManifest(manifest_raw, size, &curr);
    curr.manifestItem = (manifest_item **) realloc(curr.manifestItem, sizeof(manifest_item *) * (curr.many_Items + 1));
    asprintf(&regulized_path, "%s/%s", project_name, to_add_path);
    regulized_path = is_valid_path(regulized_path, project_name);
    if (regulized_path == NULL) { printf("File path not valid.\nPlease make sure that the file path is valid under the project_name folder\nExample:\nProject_Name\\abc\\def -> abc\\def\n");return -2; }
    strcpy(open_path, project_name);
    strcat(open_path, "/");
    strcat(open_path, regulized_path);
    status = open(open_path, O_RDONLY);
    if (status < 0) { printf("File path not valid.\n Cannot open the file %s\n",open_path);return -2; }
    new = (manifest_item *) malloc(sizeof(manifest_item));
    new->version_num = 0;
    new->hash = createBuffer();
    appendSequenceBuffer(new->hash, "0", 1);
    new->filename = createBuffer();
    appendSequenceBuffer(new->filename, regulized_path, strlen(regulized_path));
    new->filename_64 = createBuffer();
    size = 0;
    base64_encoded = base64_encode(regulized_path, strlen(regulized_path), &size);
    appendSequenceBuffer(new->filename_64, base64_encoded, size);
    for(tmp=0;tmp<curr.many_Items;tmp++){
        if(cmp_compare(curr.manifestItem[tmp],new)==0){
            printf("Error:\nFile is already being tracked!\n");
            return -1;
        }
    }
    curr.manifestItem[curr.many_Items] = new;
    curr.many_Items++;
    sort_manifest(curr.manifestItem, curr.many_Items);
    writeManifest(&manifest_raw, &curr, 0);
    return writeFile(manifest_path, manifest_raw, strlen(manifest_raw));

}

int remove_entry(char *project_name, char *path) {
    char *manifest_path;
    size_t size, temp;
    project curr;
    int status = 0;
    asprintf(&manifest_path, "%s/.Manifest", project_name);
    char *manifest_raw;
    asprintf(&path,"%s/%s",project_name,path);
    path=is_valid_path(path,project_name);
    if(path==NULL){ printf("File path not valid.\nPlease make sure that the file path is valid under the project_name folder\nExample:\nProject_Name\\abc\\def -> abc\\def\n");return -2;}
    char *base = base64_encode(path, strlen(path), &size);

    //manifest_item *new = (manifest_item*) malloc(sizeof(manifest_item));
    status = readFile(manifest_path, &manifest_raw, &size);
    if (status != 0) { return -1; } // -1 -> Manifest Reading Error
    readManifest(manifest_raw, size, &curr);
    for (temp = 0; temp < curr.many_Items; temp++) {
        if (strcmp(curr.manifestItem[temp]->filename_64->data, base) == 0) { break; }
    }
    if(temp==curr.many_Items){printf("Error remove\nFile not been tracked\n");return -2;}
    if (temp == curr.many_Items - 1) {
        curr.many_Items--;
    } else {
        curr.manifestItem[temp] = curr.manifestItem[curr.many_Items - 1];
        curr.many_Items--;
        sort_manifest(curr.manifestItem, curr.many_Items);
    }
    writeManifest(&manifest_raw, &curr, 0);
    return writeFile(manifest_path, manifest_raw, strlen(manifest_raw));
    return 0;
}

//read Configure under the current directory .configure file

int readConfigure() {
    char *buffer;
    size_t size;
    if (readFile("./.configure", &buffer, &size) == -1) { return -1; }
    ipaddr = malloc(sizeof(char) * size);
    return sscanf(buffer, "%s\n%d", ipaddr, &portno);

}

int configure(char *server_addr, char *port_no) {
    char *data;
    size_t size;
    size = asprintf(&data, "%s\n%s", server_addr, port_no);
    writeFile("./.configure", data, size);
    return 0;
}

#ifndef TEST_COMPILING
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf(PARSEERROR);
        return -1;
    }
    if (strlen(argv[1]) < 3) { printf(PARSEERROR); }

    if ((argv[1][0] + argv[1][2] * 100) != 11099) {
        if (readConfigure() == -1) {
            printf("Error Happened During Reading Configure File\n");
            return -1;
        }
    }

    switch (argv[1][0] + argv[1][2] * 100) {
        case 11099: {
            if (strcmp("configure", argv[1]) != 0 || argc != 4) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("configure");
            if(configure(argv[2], argv[3])!=0){
                printf("Error Configuring\nPossible Reasons:\n1. Program do not have write permission to current directory\n");
            }
            break;
        }
        case 10199: {
            if (strcmp("create", argv[1]) == 0 && argc == 3) {
                //printf("create");
                if (create(argv[2]) != 0) {
//                    printf("Error Create project with name %s\nPossible Reasons:\n1. Project %s already existed in sever.\n2. Error communicating with server.\n",
//                           argv[2], argv[2]);
                }
            } else if (strcmp("checkout", argv[1]) == 0 || argc == 3) {
                //printf("checkout");
                if (checkout(argv[2]) != 0) {
//                    printf("Error checkout project with name %s\nPossible Reasons:\n1. Project %s does not exist in sever.\n2. Error communicating with server.\n",
//                           argv[2], argv[2]);
                }
            } else {
                printf(PARSEERROR);
                return -1;
            }

            break;
        }
        case 10117: {
            if (strcmp("update", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("update");
            if(update(argv[2])!=0){
                //printf("Error update project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n3. Conflicts stated above.\n",argv[2],argv[2]);
            }
            break;
        }
        case 10417: {
            if (strcmp("upgrade", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("upgrade");
            if(upgrade(argv[2])!=0){
                //printf("Error upgrade project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n3. Conflicts stated above.\n",argv[2],argv[2]);
            }
            break;
        }
        case 10999: {
            if (strcmp("commit", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("commit");
            if(commit(argv[2])!=0){
                //printf("Error commit project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n3. Conflicts stated above.\n",argv[2],argv[2]);
            }
            break;
        }
        case 11612: {
            if (strcmp("push", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("push");
            if(push(argv[2])!=0){
                //printf("Error push project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n3. Conflicts stated above.\n",argv[2],argv[2]);
            }
            break;
        }
        case 11600: {
            if (strcmp("destroy", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("destroy");
            if(destroy(argv[2])!=0){
                //printf("Error destroy project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n",argv[2],argv[2]);
            }
            break;
        }
        case 10097: {
            if (strcmp("add", argv[1]) != 0 || argc != 4) {
                printf(PARSEERROR);
                return -1;
            }
            if (add(argv[2], argv[3]) != 0) {
                printf("Error Add file %s into Project %s\nPossible Reason:\n1. Project %s not existed locally.\n2. File not exist.\n",
                       argv[3], argv[2], argv[2]);
            }
            //printf("add");
            break;
        }
        case 11014: {
            if (strcmp("remove", argv[1]) != 0 || argc != 4 ) {
                printf(PARSEERROR);
                return -1;
            }
            if(remove_entry(argv[2], argv[3])!=0){printf("Error remove file %s in project %s\nPossible Reasons:\n1. Project not existed\n2.File not tracked\n",argv[3],argv[2]);};
            //printf("remove");
            break;
        }
        case 11499: {
            if (strcmp("currentversion", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("currentversion");
            if(current_version(argv[2])!=0){
                //printf("Error currentversion project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n",argv[2],argv[2]);
            }
            break;
        }
        case 11604: {
            if (strcmp("history", argv[1]) != 0 || argc != 3) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("history");
            if (history(argv[2]) != 0) {
//                printf("Error get project history with name %s\nPossible Reasons:\n1. Project %s does not exist in sever.\n2. Error communicating with server.\n",
//                       argv[2], argv[2]);
            }
            break;
        }
        case 10914: {
            if (strcmp("rollback", argv[1]) != 0 || argc != 4) {
                printf(PARSEERROR);
                return -1;
            }
            //printf("rollback");
            if(rollback(argv[2],argv[3])!=0){
                //printf("Error rollback project %s\nPossible Reasons:\n1. Project %s does not exists in server.\n2. Error communicating with server.\n3. Trying to rollback version that does not exist on server\n",argv[2],argv[2]);
            }
            break;
        }
        default: {
            printf(PARSEERROR);
            return -1;
        }
    }
    //TRACE(("size of a size_t: %d \n", sizeof(size_t)));
    return 0;
}
#endif

/*
 * TODO LIST: 1. readme modification (tar 4G & Introduction)
 *            2. testcases.txt
 *            3. testplan & code
 *            6. PATCH UPDATE
 *            8. reentrant (does not follow the procedure)
 *            9. Implement Check to be added file
 *            10. Implement check whether two lists are identical (HASH)
 *            11. changed timeout to 5 minutes
 */