#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
int DEBUG=0;
int ERROR_FLAG=0;
int myCompare(const void* a, const void* b)
{
	int curr=0;	
	const char **aa=(const char**)a;
	const char **bb=(const char**)b;
	int l1=strlen(*aa);
	int l2=strlen(*bb);
	if(DEBUG==1){
	printf("String %s %s, length%d %d\n",*aa,*bb,l1,l2);}
	while(curr!=l1||curr!=l2)
	{
		if(DEBUG==1){
		printf("Curr %d\n",curr);
		printf("%c %c\n",(*aa)[curr],(*bb)[curr]);}
		if((*aa)[curr]>='A'&& (*aa)[curr]<='Z'&& (*bb)[curr]>='a'&& (*bb)[curr]<='z'){if(DEBUG==1){printf("Exiting Comparison\n");}return -1;}
		if((*aa)[curr]>='a'&& (*aa)[curr]<='z'&& (*bb)[curr]>='A'&& (*bb)[curr]<='Z'){if(DEBUG==1){printf("Exiting Comparison\n");}return 1;}
		if((*aa)[curr]-(*bb)[curr]>0){if(DEBUG==1){printf("Exiting Comparison\n");}return -1;}
		if((*aa)[curr]-(*bb)[curr]<0){if(DEBUG==1){printf("Exiting Comparison\n");}return 1;}	//curr char in a is a not capital, another occasion is b is a later char
		// curr char in b is not capital, or a is a later char
		curr++;
	}
	if(l1>l2){if(DEBUG==1){printf("Exiting Comparison\n");}return 1;}
	if(l1<l2){if(DEBUG==1){printf("Exiting Comparison\n");}return -1;}
	if(DEBUG==1){printf("Exiting Comparison\n");}return 0;
}
int main(int argc, char **argv)
{
	int curr=0,prev=0;//current,previous position
	char *str=argv[1];
	char **tokenized,**temp_token;
	char *temp;
	int curr_size=0,thres=1000;
	tokenized=(char**)malloc(thres*sizeof(char**));
	if(argc==1){printf("Usage: pointersorter \"<inpustring>\"\n");return EXIT_SUCCESS;}
	if(argc>2){
		printf("Received More Than Two Inputs!\n");
		printf("Usage: pointersorter \"<inputstring>\"\n");
		return EXIT_SUCCESS;
	}
	//printf("%s",str);
	//printf("%d\n",strlen(str));
	while(curr!=strlen(str))
	{
		//printf("CURRstr %d %c\n",curr,str[curr]);
		if(isalpha(str[curr])==0){
			if(DEBUG==1){printf("Not Alpha\n",curr,str[curr]);}
			if(prev-curr==0){prev=curr+1;curr++;continue;}
			do{
			temp=(char*)malloc(sizeof(char)*(curr-prev+1));
			if(temp==NULL){ERROR_FLAG++;}
			else{break;}
			}while(ERROR_FLAG<=100);
			if(ERROR_FLAG==101){printf("Failed to alloc memory at line 55 for 101 times.\nCritical Failure\n Program will EXIT\n");return EXIT_SUCCESS;}
			ERROR_FLAG=0;
			temp=(char*)memcpy(temp,str+prev,(curr-prev)*sizeof(char));
			temp[curr-prev]='\0';
			//printf("%s\n",temp);
			//check if tokenized array is full
			if(curr_size>=thres){
				thres=thres*10;
				temp_token=tokenized;
				do{
				tokenized=(char**)malloc(thres*sizeof(char**));
				if(tokenized==NULL){ERROR_FLAG++;}
				else{break;}
				}while(ERROR_FLAG<=100);
				if(ERROR_FLAG==101){printf("Failed to alloc memory at line 69 for 101 times.\n Critical Failure\n Program will EXIT\n");return EXIT_SUCCESS;}
				ERROR_FLAG=0;
				tokenized=(char**)memcpy(tokenized,temp_token,curr_size*sizeof(char**));
				free(temp_token);
			}
			tokenized[curr_size]=temp;
			curr_size++;
			temp=NULL;
			prev=curr+1;
		}
		curr++;
	}
	//Since at this time, this is the end of the input string, we need to check if there is another alphabetic string in the end
	if(curr-prev!=0){
		do{
		temp=(char*)malloc(sizeof(char)*(curr-prev+1));
		if(temp==NULL){ERROR_FLAG++;}
		else{break;}
		}while(ERROR_FLAG<=100);
		if(ERROR_FLAG==101){printf("Failed to alloc memory at line 88 for 101 times.\n Critical Failure\n Program will EXIT\n");return EXIT_SUCCESS;}
		ERROR_FLAG=0;
		temp=(char*)memcpy(temp,str+prev,(curr-prev)*sizeof(char));
		temp[curr-prev]='\0';
		//printf("%s\n",temp);
		//check if tokenized array is full
		if(curr_size>=thres){
			thres=thres*10;
			temp_token=tokenized;
			do{
			tokenized=(char**)malloc(thres*sizeof(char**));
			if(tokenized==NULL){ERROR_FLAG++;}
			else{break;}
			}while(ERROR_FLAG<=100);
			if(ERROR_FLAG==101){printf("Failed to alloc memory at line 102 for 101 times.\n Critical Failure\n Program will EXIT\n");return EXIT_SUCCESS;}
			ERROR_FLAG=0;
			tokenized=(char**)memcpy(tokenized,temp_token,curr_size*sizeof(char**));
			free(temp_token);
			}
		tokenized[curr_size]=temp;
		curr_size++;
		temp=NULL;
	}
	//printf("Tokenized Size%d\n",sizeof(tokenized));
	qsort(tokenized,curr_size,sizeof(const char*),myCompare);
	//print the stuff
	for(curr=0;curr<curr_size;curr++)
	{
		printf("%s\n",tokenized[curr]);
	}
	return EXIT_SUCCESS;
}
