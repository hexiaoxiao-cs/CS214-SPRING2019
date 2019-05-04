//
// Created by Zhizhang Deng on 2019-05-04.
//

#include "serverutil.h"

#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <util.h>
void get_project_path(char* buffer, const char* project_name, size_t project_name_size, int version) {
    if (version >= 0)
        sprintf(buffer, "Projects/%.*s/%d/", (int)project_name_size, project_name, version);
    else
        sprintf(buffer, "Projects/%s/", project_name);
}

int get_latest_project_version(const char* project_name, size_t project_name_size) {
    char currentversion_path[PATH_MAX];
    char *file_data;
    size_t file_size;
    int version;

    get_project_path(currentversion_path, project_name, project_name_size, -1);
    strcat(currentversion_path, "Currentversion");
    if (readFile(currentversion_path, &file_data, &file_size) < 0) {
        TRACE(("Possible directory structure corruption, exiting... \n"));
        exit(0);    // this should never happen
    }
    sscanf(file_data, "%d", &version);
    free(file_data);
    return version;
}