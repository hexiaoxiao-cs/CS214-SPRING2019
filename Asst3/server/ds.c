#include "ds.h"
#include <pthread.h>
#include <search.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

pthread_t listener_thread_id = -1;
int bailout = 0;
pthread_mutex_t bailout_mtx = PTHREAD_MUTEX_INITIALIZER; //bailout mutex

static pthread_mutex_t hashmap_mtx = PTHREAD_MUTEX_INITIALIZER; //mutex for hashmap (project - rwlock)

//Initialize Hash Map
//Return 0 indicating success
//Otherwise indicating not success

int init_hashmap()
{
    return hcreate(10000);
}

//Get RW Lock
//Input: Project Name
//Output: a read_write lock for the specified project name
//Output: if RWlock in use

pthread_rwlock_t* get_rwlock_for_project(const char* project_name, size_t project_name_size)
{
    ENTRY e,*ep;
    ACTION act;
    pthread_rwlockattr_t attr;
    char tmp_path[PATH_MAX];
    strncpy(tmp_path, project_name, project_name_size);
    tmp_path[project_name_size] = 0;    // null terminate

    pthread_mutex_lock(&hashmap_mtx);
    e.key = (char*)tmp_path;
    ep=hsearch(e,FIND);
    if(ep==NULL){ // not found
        e.data = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
        if(pthread_rwlock_init(e.data,&attr)!=0){return NULL;} // get RWLock Failed
        hsearch(e,ENTER);
        pthread_mutex_unlock(&hashmap_mtx);
        return e.data;

    }
    else{
        pthread_mutex_unlock(&hashmap_mtx);
        return ep->data;
    }
}

