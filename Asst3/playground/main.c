//
// Created by Zhizhang Deng on 2019-05-02.
//

#include "libtar.h"

#include <fcntl.h>

#include <pthread.h>

#include <util.h>
#include <network.h>
#include <protocol.h>
#include <stdlib.h>
#include <stdio.h>

void* checkout_test(void* input) {
    const char* project_name = input;
    buffer* req = get_output_buffer_for_request(5, project_name, strlen(project_name), 0);
    buffer* response_buf;
    uint64_t tid;
    pthread_threadid_np(pthread_self(), &tid);
    parsed_response_t response;
    finalize_buffer(req);
    if (send_request("localhost", 5555, req, &response_buf) != 0) {
        printf("Thread %lld reported network error\n", tid);
        pthread_exit(0);
    }
    if (parse_response(response_buf, &response) != 0) {
        printf("Thread %lld reported parsing error\n", tid);
    }
}

int main(int argc, char* argv[]) {
    TAR* t;
    tar_open(&t, "play.tar", NULL, O_RDONLY, 0700, TAR_GNU);
    tar_extract_all(t, "./");
    tar_close(t);

    system("tar cvf files.tar a/ b/ c/");
    return 0;
}