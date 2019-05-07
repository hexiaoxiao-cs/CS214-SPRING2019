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

#include <client.h>

#include <unistd.h>

#include <time.h>

#define CONCURRENCY 50

#define T1 clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
#define T2 clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);

#define DT printf("Time: %lds : %ldms\n", diff(time1, time2).tv_sec, diff(time1, time2).tv_nsec / 1000000);

#define DTT printf("Total Time: %lds:%ldms\n", diff(time1, time2).tv_sec, diff(time1, time2).tv_nsec / 1000000);

pthread_cond_t cv;
pthread_mutex_t mtx;
pthread_mutex_t mtx_going;
pthread_barrier_t br;
uint8_t go;
char* tm_test_payload;
size_t tm_test_payload_size;
char* tt_test_payload;
size_t tt_test_payload_size;

int verbose = 0;

int running;

typedef struct {
    int id;
    buffer* (*get_buffer)(const char* proj_name);
} tpayload;

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

void* checkout_test(void* input) {
//    const char* project_name = input;
//    buffer* req = get_output_buffer_for_request(5, project_name, strlen(project_name), 0);
//    buffer* response_buf;
//    uint64_t tid;
//    pthread_threadid_np(pthread_self(), &tid);
//    parsed_response_t response;
//    finalize_buffer(req);
//    if (send_request("localhost", 5555, req, &response_buf) != 0) {
//        printf("Thread %llu reported network error\n", tid);
//        pthread_exit(0);
//    }
//    if (parse_response(response_buf, &response) != 0) {
//        printf("Thread %llu reported parsing error\n", tid);
//    }
}

void sep() {
    printf("==============================================\n");
}

void check(int ret, int fatal) {
    if (ret == 0) {
        printf("Success\n");
    } else {
        printf("Something is wrong\n");
        if (fatal == 1) {
            sep();
            exit(0);
        }
    }
}

buffer* create_generator(const char *proj_name) {
    buffer* output = get_output_buffer_for_request(0, proj_name, strlen(proj_name), 0);
    finalize_buffer(output);
    return output;
}

buffer* checkout_single_generator(const char *proj_name) {
    buffer* output = get_output_buffer_for_request(5, "single", 6, 0);
    finalize_buffer(output);
    return output;
}

buffer* checkout_generator(const char *proj_name) {
    buffer* output = get_output_buffer_for_request(5, proj_name, strlen(proj_name), 0);
    finalize_buffer(output);
    return output;
}

buffer* push_single_generator(const char *proj_name) {
    buffer* output = get_output_buffer_for_request(9, "single", 6, 1);
    appendSequenceBuffer(output, tm_test_payload, tm_test_payload_size);
    finalize_file_payload1_for_request(output);
    appendSequenceBuffer(output, tt_test_payload, tt_test_payload_size);
    finalize_buffer(output);
    return output;
}

buffer* push_generator(const char *proj_name) {
    buffer* output = get_output_buffer_for_request(9, proj_name, strlen(proj_name), 1);
    appendSequenceBuffer(output, tm_test_payload, tm_test_payload_size);
    finalize_file_payload1_for_request(output);
    appendSequenceBuffer(output, tt_test_payload, tt_test_payload_size);
    finalize_buffer(output);
    return output;
}

void* test(void *tp) {
    char proj_buffer[256];
    buffer* output, *input;
    parsed_response_t response;
    struct timespec time1, time2;
    tpayload* t = (tpayload*)tp;

    sprintf(proj_buffer, "a%d", t->id);

    // prepare payload
    output = t->get_buffer(proj_buffer);

    pthread_barrier_wait(&br);  // sync with main


    pthread_mutex_lock(&mtx);

    while (go != 1) {
        pthread_cond_wait(&cv, &mtx);   // wait for signal to go
    }

    pthread_mutex_unlock(&mtx);

    T1

    if (send_request(ipaddr, portno, output, &input) != 0) {
        printf("Network problem detected\n");
        goto end;
    }

    if (parse_response(input, &response) != 0) {
        printf("Invalid response received\n");
        goto end;
    }

    if (handle_error(&response) != 0) {
        goto end;
    }

    if (verbose)
        printf("success\n");
    end:

    T2
    DT

    free(tp);
    pthread_exit(NULL);
}

void spawn_threads(pthread_t* children, buffer* (*generator)(const char* proj)) {
    int i;
    void* dummy;
    struct timespec time1, time2;

    printf("Spawning threads\n");

    for (i = 0;i < CONCURRENCY;i++) {
        tpayload* t = malloc(sizeof(tpayload));
        t->get_buffer = generator;
        t->id = i;
        pthread_create(&children[i], NULL, test, t);
    }

    pthread_barrier_wait(&br);

    printf("Done\nExecuting\n");
    sep();

    pthread_mutex_lock(&mtx);
    go = 1;
    pthread_mutex_unlock(&mtx);
    T1
    pthread_cond_broadcast(&cv);

    for (i = 0;i < CONCURRENCY;i++)
        pthread_join(children[i], &dummy);
    sep();
    T2
    DTT
}

int main(int argc, char* argv[]) {
    pthread_condattr_t attr;
    pthread_mutexattr_t mattr;
    pthread_attr_t pattr;

    pthread_t children[CONCURRENCY];

    struct timespec time1, time2;

    void* dummy;

    int i;

    pthread_condattr_init(&attr);
    pthread_cond_init(&cv, &attr);
    pthread_mutexattr_init(&mattr);
    pthread_mutex_init(&mtx, &mattr);

    pthread_barrier_init(&br, NULL, CONCURRENCY + 1);

    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_DETACHED);

    if (!(argc == 3 || argc == 4)) {
        return 0;
    }

    if (argc == 4) {
        if (strcmp(argv[3], "-v") == 0)
            verbose = 1;
    }

    configure(argv[1], argv[2]);
    if (readConfigure() == -1) {
        printf("Something is wrong with the .Configure file\n");
        return 0;
    }

    readFile("tm_test_payload", &tm_test_payload, &tm_test_payload_size);
    readFile("tt_test_payload", &tt_test_payload, &tt_test_payload_size);


    sep();
    printf("Creating test project \"single\"\n");
    sep();
    check(create("single"), 0);
    sep();

    sep();
    printf("Creating multiple projects (Multi-threaded)\n");
    sep();
    spawn_threads(children, create_generator);
    sep();

    printf("Pushing single project multithreaded\n");
    sep();
    spawn_threads(children, push_single_generator);
    sep();

    printf("Pushing multiple project multithreaded\n");
    sep();
    spawn_threads(children, push_generator);
    sep();

    printf("Checking out \"single\" project multithreaded\n");
    sep();
    spawn_threads(children, checkout_single_generator);
    sep();

    printf("Checking out multiple project multithreaded\n");
    sep();
    spawn_threads(children, checkout_generator);
    sep();
    return 0;
}