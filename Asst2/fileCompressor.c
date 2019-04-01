#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<regex.h>
#include<dirent.h>
#include<errno.h>
#include<sys/types.h>
#include"interface.h"

int isFile(const char* name)
{
	DIR* directory = opendir(name);
	if(directory != NULL)
	{
		closedir(directory);
		return 0;
	}
	if(errno == ENOTDIR)
	{
		return 1;
	}
	return -1;
}





int main(int argc, char* argv[])
{
	int c;
	int buildCodeBook=0,compress=0,decompress=0,fs=0,recursive=0,hascodebook=0;
	char *filename,*codebook;
	extern char *optarg;
	extern int optind, optopt, opterr;
	int reti;
	regex_t regex;
	reti=regcomp(&regex,".*\\.hcz",0);
	if(reti){printf("regex ERROR");}
	while ((c = getopt(argc, argv, "bcdR")) != -1) {
	    switch(c) {
	        case 'b':
		        buildCodeBook=1;
		        break;
		case 'c':
		        compress=1;
		        break;
		case 'd':
		        decompress=1;
		        break;
		case 'R':
			recursive=1;
			break;
		case '?':
			printf("unknown arg %c\n", optopt);

		default:
			printf("Usage: fileCompressor <flag> <path or file> |codebook|\n");
			return 2;
		}
	}
	if(argv[optind]==NULL){printf("Missing Target\n Usage: fileCompressor <flag> <path or file> |codebook|\n");return 2;}
	filename=argv[optind];
	reti=regexec(&regex,filename,0,NULL,0);
	fs=isFile(filename);
	if(decompress==1&&fs==1&&reti==REG_NOMATCH){printf("option d: The Target should be a directory or an .hcz file\n");return 2;}
	if((recursive==1&&fs==1)||(recursive==0&&fs == 0)){printf("option R: Only for Directories\n");return 2;}
	if (argv[optind+1]!=NULL) {
		codebook=argv[optind+1];
		hascodebook=1;
	}
	else{codebook="./HuffmanCodebook";
		printf("Using Default Codebook in ./HuffmanCodebook\n");
		
		if(isFile(codebook)!=1&&decompress==1){printf("Default Codebook Not Found!\nProgram will EXIT!\n");return 2;}
	}
	//printf("%d",hascodebook);
	if(buildCodeBook==1){
		if(recursive==1){doShits(filename,0,NULL,1);
		}
		else{
			doSingleShit(filename,0,NULL,1);
		}
	}
	if(compress==1){
		if(recursive==1){doShits(filename,hascodebook,codebook,0);
		}
		else{
			doSingleShit(filename,hascodebook,codebook,0);
		}
	}
	if(decompress==1){
		if(recursive==1){undoShits(filename,codebook);}
		else{
			undoSingleShit(filename,codebook);}
	}

	return 0;
}
