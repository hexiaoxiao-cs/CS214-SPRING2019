//
// Created by Xiaoxiao He on 5/2/2019.
//

#ifndef WTF_LIBTAR_H
#define WTF_LIBTAR_H

#include <unistd.h>

#define TAR_GNU 1

typedef struct TAR TAR;

typedef int (*openfunc_t)(const char *, int, ...);
typedef int (*closefunc_t)(int);
typedef ssize_t (*readfunc_t)(int, void *, size_t);
typedef ssize_t (*writefunc_t)(int, const void *, size_t);

typedef struct
{
    openfunc_t openfunc;
    closefunc_t closefunc;
    readfunc_t readfunc;
    writefunc_t writefunc;
}
tartype_t;

int tar_open(TAR **t, char *pathname, tartype_t *type,
             int oflags, int mode, int options);
int tar_append_file(TAR *t, char *realname, char *savename);
int tar_extract_all(TAR *t, char *prefix);
int tar_close(TAR *t);

#endif //WTF_LIBTAR_H
