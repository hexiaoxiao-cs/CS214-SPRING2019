#include "ds.h"
#include <pthread.h>
#include <search.h>
#include <stdlib.h>

pthread_t listener_thread_id;
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

pthread_rwlock_t* get_rwlock_for_project(const char* project_name)
{
    ENTRY e,*ep;
    ACTION act;
    pthread_rwlockattr_t attr;
    pthread_mutex_lock(&hashmap_mtx);
    e.key=project_name;
    ep=hsearch(e,FIND);
    if(ep->data==NULL){ // not found
        ep->data = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
        pthread_rwlockattr_init(&attr);
        pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
        if(pthread_rwlock_init(ep->data,&attr)!=0){return NULL;} // get RWLock Failed
        hsearch(*ep,ENTER);
        return ep->data;

    }
    else{
        return ep->data;
    }
}

