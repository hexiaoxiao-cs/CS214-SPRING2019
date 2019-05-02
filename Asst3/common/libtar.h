//
// Created by Xiaoxiao He on 5/2/2019.
//

#ifndef WTF_LIBTAR_H
#define WTF_LIBTAR_H

int tar_open(TAR **t, char *pathname, tartype_t *type,
             int oflags, int mode, int options);
int tar_append_file(TAR *t, char *realname, char *savename);
int tar_extract_all(TAR *t, char *prefix);


#endif //WTF_LIBTAR_H
