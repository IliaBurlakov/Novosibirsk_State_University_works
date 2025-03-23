#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "header.h"

#define MAX_PATH_COMPONENTS 256
#define FULL_PERMISSIONS_MASK 07777

void getFullPath(const char *dir, const char *name, char *result) {
    size_t len = strlen(dir);
    if (dir[len - 1] == '/') {
        snprintf(result, FILENAME_MAX, "%s%s", dir, name);
        return;
    } 
    snprintf(result, FILENAME_MAX, "%s/%s", dir, name);
}

int openSrcFile(const char *srcPath) {
    int fd = open(srcPath, O_RDONLY);
    if (fd == ERROR) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Opening source file for reading: %s", srcPath);
        perror(errorMessage);
    }
    return fd;
}

int openDstFile(const char *dstPath, mode_t mode) {
    int fd = open(dstPath, O_WRONLY | O_CREAT, mode);
    if (fd == ERROR) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Opening destination file for writing: %s", dstPath);
        perror(errorMessage);
    }
    return fd;
}

char* readFileContents(int fd, size_t size, const char *srcPath) {
    char *buf = malloc(size);
    if (buf == NULL) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Allocating buffer for file: %s", srcPath);
        perror(errorMessage);
        return NULL;
    }
    ssize_t bytesRead = read(fd, buf, size);
    if (bytesRead == ERROR || (size_t)bytesRead != size) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Reading file: %s", srcPath);
        perror(errorMessage);
        free(buf);
        return NULL;
    }
    return buf;
}

void reverseBuffer(char *buf, size_t bufSize) {
    for (size_t i = 0; i < bufSize / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[bufSize - i - 1];
        buf[bufSize - i - 1] = tmp;
    }
}

int processRegularFile(const char *srcPath, const char *dstPath, struct stat st) {
    int src = openSrcFile(srcPath);
    if (src == ERROR)
        return ERROR;

    int dst = openDstFile(dstPath, st.st_mode & FULL_PERMISSIONS_MASK);
    if (dst == ERROR) {
        close(src);
        return ERROR;
    }

    char *buf = readFileContents(src, st.st_size, srcPath);
    if (buf == NULL) {
        close(src);
        close(dst);
        return ERROR;
    }

    reverseBuffer(buf, st.st_size);

    ssize_t bytesWritten = write(dst, buf, st.st_size);
    if (bytesWritten == ERROR || (off_t)bytesWritten != st.st_size) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage),"Writing file: %s",dstPath);
        perror(errorMessage);
        free(buf);
        close(src);
        close(dst);
        return ERROR;
    }
    free(buf);
    close(src);
    close(dst);
    return SUCCESS;
}
int getReversedPath(const char *originalPath, char *reversedPath) {
    size_t pathLen = strlen(originalPath);
    if (pathLen == 1) {
        strcpy(reversedPath, originalPath);
        return SUCCESS;
    }
    char originalPathCopy[FILENAME_MAX];
    strcpy(originalPathCopy, originalPath);
    originalPathCopy[pathLen] = '\0';

    if (originalPathCopy[pathLen - 1] == '/') {
        originalPathCopy[pathLen - 1] = '\0';
        pathLen--;
    }

    char *components[MAX_PATH_COMPONENTS];
    int componentsCount = 0;
    char *token = strtok(originalPathCopy, "/");
    
    while (token != NULL && componentsCount < MAX_PATH_COMPONENTS) {
        components[componentsCount++] = token;
        token = strtok(NULL, "/");
    }
    if (token != NULL){
        fprintf(stderr, "Path has more than %d components\n", MAX_PATH_COMPONENTS);
        return ERROR;
    }
    
    reversedPath[0] = '\0';
    
    for (int i = 0; i < componentsCount; i++) {
        size_t componentSize = strlen(components[i]);
        reverseBuffer(components[i], componentSize);
        if (i != 0 ) {
            strcat(reversedPath, "/");
        }
        strcat(reversedPath, components[i]);
    }
    return SUCCESS;
}

int processEntry(const char *srcDir, const char *dstDir, const struct dirent *ent) {
    char srcPath[FILENAME_MAX];
    getFullPath(srcDir, ent->d_name, srcPath);

    char reversedName[FILENAME_MAX];
    strncpy(reversedName, ent->d_name, FILENAME_MAX - 1);
    reversedName[FILENAME_MAX - 1] = '\0';
    size_t reversedNameLength = strlen(reversedName);
    reverseBuffer(reversedName, reversedNameLength);

    char dstPath[FILENAME_MAX];
    getFullPath(dstDir, reversedName, dstPath);

    struct stat entryStat;
    int lstatReturnValue = lstat(srcPath, &entryStat);
    if (lstatReturnValue == ERROR) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Lstat source path: %s", srcPath);
        perror(errorMessage);
        return ERROR;
    }

    if (S_ISDIR(entryStat.st_mode) == 1) {
        int reverseDirReturnValue = reverseDirectory(srcPath, dstPath);
        if (reverseDirReturnValue == ERROR)
            return ERROR;
    } 
    
    if (S_ISREG(entryStat.st_mode)){
        int processRegFileReturnValue = processRegularFile(srcPath, dstPath, entryStat);
        if (processRegFileReturnValue == ERROR)
            return ERROR;
    }

    return SUCCESS;
}


int reverseDirectory(const char *srcDir, const char *dstDir) {
    struct stat st;
    int statReturnValue = stat(srcDir, &st);
    if (statReturnValue == ERROR) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Stat source directory: %s", srcDir);
        perror(errorMessage);
        return ERROR;
    }
    int mkdirReturnValue = mkdir(dstDir, st.st_mode & FULL_PERMISSIONS_MASK);
    if (mkdirReturnValue == ERROR && errno != EEXIST) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Create destination directory: %s", dstDir);
        perror(errorMessage);
        return ERROR;
    }

    DIR *dir = opendir(srcDir);
    if (dir == NULL) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Open source directory: %s", srcDir);
        perror(errorMessage);
        return ERROR;
    }
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        int comparisonWithCurrentDir = strcmp(ent->d_name, ".");
        int comparisonWithUpperDir = strcmp(ent->d_name, "..");
        if (comparisonWithCurrentDir == 0 || comparisonWithUpperDir == 0)
            continue;
        int processEntryReturnValue = processEntry(srcDir, dstDir, ent);
        if (processEntryReturnValue == ERROR) {
            closedir(dir);
            return ERROR;
        }
    }
    closedir(dir);
    return SUCCESS;
}