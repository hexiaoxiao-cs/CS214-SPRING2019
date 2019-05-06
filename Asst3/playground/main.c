//
// Created by Zhizhang Deng on 2019-05-02.
//

#include "libtar.h"

#include <fcntl.h>

int main(int argc, char* argv[]) {
    TAR* t;
    tar_open(&t, "play.tar", NULL, O_RDONLY, 0700, TAR_GNU);
    tar_extract_all(t, "./");
    tar_close(t);
    return 0;
}