//
// Created by Zhizhang Deng on 2019-05-02.
//

#include "libtar.h"

#include <fcntl.h>

int main(int argc, char* argv[]) {
    TAR* t;
    tar_open(&t, "tmp.tar", NULL, O_RDONLY | O_CREAT, 0700, TAR_GNU);
    //tar_append_file(t, "Makefile", "./a/Makefile");
    tar_extract_all(t, "tmp/");
    tar_close(t);
    return 0;
}