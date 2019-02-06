#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
int DEBUG=1;
int myCompare(const void* a, const void* b)
{
	int curr=0;
	int l1=strlen((const char*)a);
	int l2=strlen((const char*)b);
	const char **aa=(const char**)a;
	const char **bb=(const char**)b;
//	printf("length%d %d\n",l1,l2);
	while(curr!=l1||curr!=l2)
	{
//		printf("%c %c\n",*aa[curr],*bb[curr]);
		if(*aa[curr]>='A'&&*aa[curr]<='Z'&&*bb[curr]>='a'&&*bb[curr]<='z'){return -1;}
		if(*aa[curr]>='a'&&*aa[curr]<='z'&&*bb[curr]>='A'&&*bb[curr]<='Z'){return 1;}
		if(*aa[curr]-*bb[curr]>0){return -1;}
		else {return 1;}	//curr char in a is a not capital, another occasion is b is a later char
		// curr char in b is not capital, or a is a later char
		curr++;
	}
	if(l1>l2){return 1;}
	if(l1<l2){return -1;}
	return 0;
}
int main(int argc, char **argv)
{
	int curr=0,prev=0;//current,previous position
	char *str=argv[1];
	char **tokenized,**temp_token;
	char *temp;
	int curr_size=0,thres=1000;
	tokenized=(char**)malloc(thres*sizeof(char**));
	if(argc>2){
		printf("Received More Than Two Inputs!\n");
		return;
	}
	//printf("%s",str);
	//printf("%d\n",strlen(str));
	while(curr!=strlen(str))
	{
		//printf("CURRstr %d %c\n",curr,str[curr]);
		if(isalpha(str[curr])==0){
			//if(DEBUG==1){printf("Not Alpha\n",curr,str[curr]);}
			if(prev-curr==0){prev=curr+1;curr++;continue;}
			temp=(char*)malloc(sizeof(char)*(curr-prev+1));
			temp=(char*)memcpy(temp,str+prev,(curr-prev)*sizeof(char));
			temp[curr-prev]='\0';
			//printf("%s\n",temp);
			//check if tokenized array is full
			if(curr_size>=thres){
				thres=thres*10;
				temp_token=tokenized;
				tokenized=(char**)malloc(thres*sizeof(char**));
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
	if(curr-prev!=0){
		temp=(char*)malloc(sizeof(char)*(curr-prev+1));
		temp=(char*)memcpy(temp,str+prev,(curr-prev)*sizeof(char));
		temp[curr-prev]='\0';
		//printf("%s\n",temp);
		//check if tokenized array is full
		if(curr_size>=thres){
			thres=thres*10;
			temp_token=tokenized;
			tokenized=(char**)malloc(thres*sizeof(char**));
			tokenized=(char**)memcpy(tokenized,temp_token,curr_size*sizeof(char**));
			free(temp_token);
			}
		tokenized[curr_size]=temp;
		curr_size++;
		temp=NULL;
	}
	qsort(tokenized,curr_size,sizeof(const char*),myCompare);
	//print the stuff
	for(curr=0;curr<curr_size;curr++)
	{
		printf("%s\n",tokenized[curr]);
	}
	return EXIT_SUCCESS;
}
