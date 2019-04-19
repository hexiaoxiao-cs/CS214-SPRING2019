#include "ds.h"

static pthread_mutex_t hashmap_mtx = PTHREAD_MUTEX_INITIALIZER; //mutex for hashmap (project - rwlock)
