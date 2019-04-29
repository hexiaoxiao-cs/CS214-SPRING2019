#include "client.h"
#include "util.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>

int main() {
    TRACE(("size of a size_t: %d \n", sizeof(size_t)));
    return 0;
}