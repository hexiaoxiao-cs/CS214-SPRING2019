//
// Created by Zhizhang Deng on 2019-05-02.
//

#include "clientutil.h"

#include <stdlib.h>
#include <string.h>

int cmp_compare(const void* a_, const void* b_) {
    manifest_item* a = *((manifest_item**)a_);
    manifest_item* b = *((manifest_item**)b_);
    if (a->filename->size != b->filename->size) {
        return a->filename->size - b->filename->size;
    } else {
        return memcmp(a->filename->data, b->filename->data, a->filename->size);
    }
}

void sort_manifest(manifest_item** items, size_t len) {
    qsort(items, len, sizeof(manifest_item*), cmp_compare);
}