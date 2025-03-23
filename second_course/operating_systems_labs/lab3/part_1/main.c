#include <stdio.h>
#include <sys/stat.h>
#include "header.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return ERROR;
    }

    struct stat st;
    int statReturnValue = stat(argv[1], &st);
    if (statReturnValue == ERROR) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Check input directory: %s", argv[1]);
        perror(errorMessage);
        return ERROR;
    }
    
    if (S_ISDIR(st.st_mode) != 1){
        fprintf(stderr, "%s is not a directory\n", argv[1]);
        return ERROR;
    }

    char reversedPath[FILENAME_MAX];
    int getRevPathReturnValue = getReversedPath(argv[1], reversedPath);
    if (getRevPathReturnValue == ERROR){
        return ERROR;
    }

    int reverseDirReturnValue = reverseDirectory(argv[1], reversedPath);
    if (reverseDirReturnValue == ERROR) {
        fprintf(stderr, "Error while reversing directory\n");
        return ERROR;
    }
    return SUCCESS;
}