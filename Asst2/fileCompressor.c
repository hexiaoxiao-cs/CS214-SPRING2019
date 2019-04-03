#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include "interface.h"

int isFile(const char *name) {
    DIR *directory = opendir(name);
    if (directory != NULL) {
        closedir(directory);
        return 0;
    }
    if (errno == ENOTDIR) {
        return 1;
    }
    return -1;
}


int main(int argc, char *argv[]) {
    int c;
    int buildCodeBook = 0, compress = 0, decompress = 0, fs = 0, recursive = 0, hascodebook = 0;
    char *filename, *codebook;
    int reti;
    regex_t regex;
    reti = regcomp(&regex, ".*\\.hcz", 0); //init Regex Engine
    //using regex match to ensure that decompressing file is a file ending with .hcz
    if (reti) { printf("regex ERROR"); }
    while ((c = getopt(argc, argv, "bcdR")) != -1) { //using getopt for getting operands
        switch (c) {
            case 'b':
                buildCodeBook = 1;
                break;
            case 'c':
                compress = 1;
                break;
            case 'd':
                decompress = 1;
                break;
            case 'R':
                recursive = 1;
                break;
            case '?':
                printf("unknown arg %c\n", optopt);

            default:
                printf("Usage: fileCompressor <flag> <path or file> |codebook|\n");
                return 2;
        }
    }
    if (buildCodeBook == 0 && compress == 0 && decompress == 0) {
        printf("Need Flags\n Usage: fileCompressor <flag> <path or file> |codebook|\n");
        return 2;
    } //no option selected
    if (argv[optind] == NULL) {
        printf("Missing Target\n Usage: fileCompressor <flag> <path or file> |codebook|\n");
        return 2;
    }//no path or file given
    filename = argv[optind];
    reti = regexec(&regex, filename, 0, NULL, 0);
    fs = isFile(filename);
    if (decompress == 1 && fs == 1 && reti == REG_NOMATCH) {
        printf("option d: The Target should be a directory or an .hcz file\n");
        return 2;
    }//not a file end with .hcz
    if ((recursive == 1 && fs == 1) || (recursive == 0 && fs == 0)) {
        printf("Please use -R for directory\n");
        return 2;
    } //Ensure that when dealing with folder the recursive flag is set
    if (argv[optind + 1] != NULL) {
        codebook = argv[optind + 1];
        if (isFile(codebook) == 1) {
            hascodebook = 1;
        }
    }//checking whether user input a codebook
    else {
        codebook = "./HuffmanCodebook"; //if user does not input a codebook use Default Codebook
        printf("Using Default Codebook path in ./HuffmanCodebook\n");
    }
    //printf("%d",hascodebook);
    if (decompress == 1 && isFile(codebook) != 1) {
        printf("Default Codebook Not Found!\nProgram will EXIT!\n");
        return 2;
    }
    //When decompressing and a given codebook, if the codebook path is given incorrect, the program will exit.
    //execute:
    if (buildCodeBook == 1) {
        if (recursive == 1) {
            doShits(filename, 0, codebook, 1);
        } else {
            doSingleShit(filename, 0, codebook, 1);
        }
    }
    if (compress == 1) {
        if (recursive == 1) {
            if (strchr(filename, '"')) {
                printf("We do not support path that contains \"\n");
                return 1;
            }
            doShits(filename, hascodebook, codebook, 0);
        } else {
            doSingleShit(filename, hascodebook, codebook, 0);
        }
    }
    if (decompress == 1) {
        //printf("DECOMPRESS\n");
        if (recursive == 1) {
            if (strchr(filename, '"')) {
                printf("We do not support path that contains \"\n");
                return 1;
            }
            undoShits(filename, codebook);
        }
        else {
            undoSingleShit(filename, codebook);
        }
    }
    remove("output.tmp");
    return 0;
}
