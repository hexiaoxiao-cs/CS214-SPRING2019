#ifndef DS_H
#define DS_H

#include <pthread.h>

extern pthread_t listener_thread_id;
extern int bailout;
extern pthread_mutex_t bailout_mtx;

int init_hashmap();
pthread_rwlock_t* get_rwlock_for_project(const char* project_name);

#endif
