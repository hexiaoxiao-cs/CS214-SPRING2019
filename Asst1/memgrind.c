#include <stdio.h>
#include <time.h>	//Provide time seeds for randomization
#include <stdlib.h>	//rand()/srand()
#include "mymalloc.h"	//Replace default malloc/free

//Randomly return 0 or 1
int dice() {
	return rand() % 2;
}
//Randomly return 1 to 64
int better_dice() {
	return rand() % 64 + 1;
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
		addr=1;
		printf("%d\n",(int)(addr-base));
		free(addr);
	}
	{
		//B
		void* array[50];
		for(i = 0;i < 50;i++) {
			array[i] = malloc(1);
			printf("%d\n",(int)( array[i]-(void*)base));
		}
		for(i = 0;i < 50;i++) {
			free(array[i]);
		}
	}
	{
		//C
		unsigned int counter = 0;
		void* array[50];
		int end_idx = 0;
		while(counter < 50) {
			int choice = dice();
			if(choice == 1) {
				array[end_idx++] = malloc(1);
				if(end_idx-1!=0){
				printf("%d\n",(int)(array[end_idx-1]-(void*)base));}
				counter++;
			} else {
				if(end_idx == 0)
					continue;
				free(array[--end_idx]);
			}
		}
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
				array[end_idx++] = malloc(better_dice());
				if(end_idx-1!=50){
				printf("%d\n",(int)(array[end_idx-1]-(void*)base));}
				counter++;
			} else {
				if(end_idx == 0)
					continue;
				free(array[--end_idx]);
			}
		}
		while(end_idx > 0) {
			free(array[--end_idx]);
		}

	}
}
