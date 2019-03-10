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
    return rand() % (maximum) + 1;
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
		if(ptrarr[temp].ptr!=NULL){
			currptr=ptrarr[temp].ptr;
			currsize=ptrarr[temp].allocated_size;
			while(currsize!=0)
			{
				memset(currptr++,(char)rand(),1);
				currsize--;
				//printf("!! At %p written 1 byte of data !!\n",currptr);
			}
		}
	}
}

int compare_func(const void* a, const void* b) {
    return ((idx_ptr*)a)->idx < ((idx_ptr*)b)->idx;
}

void xjbfuckmemory() {
    idx_ptr ptrs[4096];
    void* tmp = NULL;
    int i = 0, counter = 0;
    int size = 0;

    memset(ptrs, 0, 4096 * sizeof(idx_ptr));    //Zero out memory

    while((size = way_better_dice(100), tmp = malloc(size)) != NULL) {
        printf("!! Allocated ptrs[%d]: %d bytes !!\n", counter, size);
        ptrs[counter].ptr = tmp;
        ptrs[counter].allocated_size = size;
        counter++;
    }

    xjbwrite(ptrs);

    for(i = 0;i<4096;i++) {
        ptrs[i].idx = way_better_dice(5000);    //Randomly distribute index
        printf("!! Randomly distributed idx[%d]: %d !!\n", i, ptrs[i].idx);
    }

    qsort(ptrs, 4096, sizeof(idx_ptr), compare_func);

    for(i = 0;i<4096;i++) {
        if(ptrs[i].ptr != NULL)
            printf("!! Randomly free idx[%d] !!\n", i);
        free(ptrs[i].ptr);
    }
}

void caosimemory(){
    idx_ptr ptrs[4096];
    void* tmp = NULL;
    int i = 0, counter = 0,j = 0;
    int size = 0;

    memset(ptrs, 0, 4096 * sizeof(idx_ptr));    //Zero out memory

    while((size = way_better_dice(100), tmp = malloc(size)) != NULL) {
        printf("!! Allocated ptrs[%d]: %d bytes !!\n", counter, size);
        ptrs[counter].ptr = tmp;
        ptrs[counter].allocated_size = size;
        counter++;
    }

    xjbwrite(ptrs);

    while(1){
        for(i = 0;i<4096;i++) {
            ptrs[i].idx = way_better_dice(5000);    //Randomly distribute index
            //printf("!! Randomly distributed idx[%d]: %d !!\n", i, ptrs[i].idx);
        }

        qsort(ptrs, 4096, sizeof(idx_ptr), compare_func);
        j=0;
        for(i=0;i<50;)
        {
            if(j>4095){return;}
            if(ptrs[j].ptr!=NULL)
            {
                printf("!! Randomly free idx: [%p,%d] !!\n",ptrs[j].ptr, ptrs[j].allocated_size);
                free(ptrs[j].ptr);
                ptrs[j].ptr=NULL;
                i++;
            }
            j++;
        }
        j=0;
        for(i=0;i<25;)
        {
            if(j>4095){return;}
            if(ptrs[j].ptr==NULL){
                ptrs[j].allocated_size=way_better_dice(100);
                ptrs[j].ptr=malloc(ptrs[j].allocated_size);
                i++;
            }
            j++;
        }

    }

}

int main(int argc, char** argv) {
    unsigned int i, j;
    void *addr;
    clock_t time_stats[7];
    clock_t begin;
    for(i = 0;i < 6;i++) {
        time_stats[i] = 0;  //Initialize to zero
    }

    //Initialize seeds
    srand(time(NULL));
    for(j = 0;j < 100;j++) {
        goto F;
        //A
        begin = clock();
        for(i = 0;i < 150;i++) {
            addr=malloc(1);
            free(addr);
        }
        time_stats[0] += clock() - begin;

        {
            //B
            void* array[50];
            begin = clock();
            printf("[memgrind]: B allocating... \n");
            for(i = 0;i < 50;i++) {
                array[i] = malloc(1);
            }
            printf("[memgrind]: B deallocating... \n");
            for(i = 0;i < 50;i++) {
                free(array[i]);
            }
            time_stats[1] += clock() - begin;
            fflush(stdout);
        }
        {
            //C
            unsigned int counter = 0;
            void* array[50];
            int end_idx = 0;
            begin = clock();
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
            time_stats[2] += clock() - begin;
        }
        {
            //D
            unsigned int counter = 0;
            void* array[50];
            int end_idx = 0;
            begin = clock();

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
            time_stats[3] += clock() - begin;
        }
        {
            //E
            begin = clock();
            for(int i = 0;i < 2;i++) {
                xjbfuckmemory();
                printf("===============================\n");
            }
            time_stats[4] += clock() - begin;
        }
F:
        {
            //F
            begin = clock();
            caosimemory();
            time_stats[5] += clock() - begin;
        }
    }
    for(i = 0;i < 6;i++) {
        printf("Cost of %c: %.17g\n", 65 + i, time_stats[i] / 100.0 / CLOCKS_PER_SEC / 1000 );  //Initialize to zero
    }
}
