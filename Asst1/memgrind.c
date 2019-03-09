#include <stdio.h>
#include <string.h>
#include <time.h>   //Provide time seeds for randomization
#include <stdlib.h> //rand()/srand()
#include "mymalloc.h"   //Replace default malloc/free

typedef struct {
    void* ptr;
    int allocated_size;
    int idx;
} idx_ptr;

//Randomly return 0 or 1
int dice() {
    return rand() % 2;
}
//Randomly return 0 to a specific number
int way_better_dice(int maximum) {
    return rand() % (maximum + 1);
}
//Randomly return 1 to 64
int better_dice() {
    return rand() % 64 + 1;
}

//random (xiajiba) write data into allocated place

void xjbwrite(idx_ptr* ptrarr)
{
	int temp=0;
	char xjb;
	int currsize;
	void* currptr;
	for(temp=0;temp<4096;temp++)
	{
		if(ptrarr[temp]->ptr!=NULL){
			currptr=ptrarr[temp]->ptr;
			currsize=ptrarr[temp]->size;
			while(currsize!=0)
			{
				memset(currptr++,(char)rand(),1);
				currsize--;
				printf("!! At %x written 1 byte of data!!\n",currptr);
			}
		}
	}
}

int compare_func(const idx_ptr* a, const idx_ptr* b) {
    return a->idx < b->idx;
}

void* xjbfuckmemory() {
    idx_ptr ptrs[4096];
    void* tmp = NULL;
    int i, counter = 0;
    int size = 0;

    memset(ptrs, 0, 4096 * sizeof(idx_ptr));    //Zero out memory

    while((size = way_better_dice(100), tmp = malloc(size)) != NULL) {
        printf("!! Allocated ptrs[%d]: %d bytes !!\n", counter, size);
        ptrs[counter].ptr = tmp;
        ptrs[counter].allocated_size = size;
        counter++;
    }

    xjbwrite(ptrs);

    while(ptrs[i].ptr != NULL) {
        ptrs[i].idx = way_better_dice(5000);    //Randomly distribute index
        printf("!! Randomly distributed idx[%d]: %d !!\n", i, ptrs[i].idx);
        i++;
    }

    qsort(ptrs, 4096, sizeof(idx_ptr), compare_func);

    for(i = 0;i<4096;i++) {
        printf("!! Randomly free idx[%d] !!\n", i);
        free(ptrs[i].ptr);
    }
}

int main(int argc, char** argv) {
    unsigned int i;
    //Initialize seeds
    srand(time(NULL));
    void *addr;
    void *base;
    base=malloc(1);
    free(base);
    for(i = 0;i < 150;i++) {
        addr=malloc(1);
        free(addr);
    }
    {
        //B
        void* array[50];
        printf("[memgrind]: B allocating... \n");
        for(i = 0;i < 50;i++) {
            array[i] = malloc(1);
        }
        printf("[memgrind]: B deallocating... \n");
        for(i = 0;i < 50;i++) {
            free(array[i]);
        }
        fflush(stdout);
    }
    {
        //C
        unsigned int counter = 0;
        void* array[50];
        int end_idx = 0;
        while(counter < 50) {
            int choice = dice();
            if(choice == 1) {
                printf("[memgrind]: C allocating... \n");
                array[end_idx++] = malloc(1);
                counter++;
            } else {
                if(end_idx == 0)
                    continue;
                printf("[memgrind]: C deallocating... \n");
                free(array[--end_idx]);
            }
        }
        printf("[memgrind]: C final deallocating... \n");
        while(end_idx > 0) {
            free(array[--end_idx]);
        }
    }
    {
        //D
        unsigned int counter = 0;
        void* array[50];
        int end_idx = 0;
        while(counter < 50) {
            int choice = dice();
            if(choice == 1) {
                printf("[memgrind]: D allocating... \n");
                array[end_idx++] = malloc(better_dice());
                counter++;
            } else {
                if(end_idx == 0)
                    continue;
                printf("[memgrind]: D deallocating... \n");
                free(array[--end_idx]);
            }
        }
        printf("[memgrind]: D final deallocating... \n");
        while(end_idx > 0) {
            free(array[--end_idx]);
        }

    }
    {
        //E
        void* data = malloc(50);
        void* data2 = malloc(50);
        void* data3 = malloc(50);
        free(data2);
        free(data);
        free(data3);
    }
}
