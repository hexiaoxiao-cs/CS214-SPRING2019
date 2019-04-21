#include "ds.h"
#include <pthread.h>
#include <search.h>

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

pthread_rwlock_t* get_rwlock_for_project(const char* project_name)
{
    ENTRY e,*ep;
    ACTION act;

    e.key=project_name;
    ep=hsearch(e);
    if(ep->data==NULL){
        if(pthread_rwlock_init(ep->data,pthread_rwlockattr_setkind_np())!=0){return NULL;}
        else {
            hsearch(ep);
            return ep->data;
        }
    }
}

