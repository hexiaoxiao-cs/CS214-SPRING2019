//
// Created by Zhizhang Deng on 2019-05-04.
//

#ifndef WTF_SERVERUTIL_H
#define WTF_SERVERUTIL_H

#include <stdio.h>

/*
 * This function construct a project (version) spath relative to cwd
 *
 * Note:
 *  1. buffer has to have PATH_MAX bytes to hold the output
 *  2. when version is less than 0, no version path will be given (only output the project path)
 */
void get_project_path(char* buffer, const char* project_name, size_t project_name_size, int version);
int get_latest_project_version(const char* project_name, size_t project_name_size);

#endif //WTF_SERVERUTIL_H
