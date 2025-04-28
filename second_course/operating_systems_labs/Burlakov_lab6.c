#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PAGE_SIZE 4096
#define SUCCESS 0
#define ERROR -1

void runReaderProcess(unsigned int *shared_mem) {
    int pid = getpid();
    printf("Reader (child) PID: %d\n", pid);
    const size_t maxElements = PAGE_SIZE / sizeof(unsigned int);
    unsigned int expectedValue = 0;
    while (1) {
        for (size_t i = 0; i < maxElements; i++) {
            if (shared_mem[i] != expectedValue)
                printf("Reader Error: Expected %u, Got %u\n", expectedValue, shared_mem[i]);

            expectedValue++;
            if (expectedValue == UINT_MAX)
                expectedValue = 0;
        }
    }
}

void runWriterProcess(unsigned int *shared_mem) {
    int pid = getpid();
    printf("Writer (parent) PID: %d\n", pid);
    unsigned int counter = 0;
    const size_t maxElements = PAGE_SIZE / sizeof(unsigned int);

    while (1) {
        for (size_t i = 0; i < maxElements; i++) {
            shared_mem[i] = counter++;
            if (counter == UINT_MAX)
                counter = 0;
        }
    }
}

int main() {
    int fdForMmap = -1;
    off_t offsetForMmap = 0;
    unsigned int *sharedMem = mmap(
        NULL,
        PAGE_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        fdForMmap,
        offsetForMmap
    );

    if (sharedMem == MAP_FAILED) {
        perror("mmap in main");
        return ERROR;
    }

    pid_t pid = fork();
    if (pid == ERROR) {
        perror("fork in main");
        return ERROR;
    }
    if (pid == 0) 
        runReaderProcess(sharedMem);

    if (pid > 0)
        runWriterProcess(sharedMem);

    int status;
    pid_t waitReturnValue = wait(&status);
    if (waitReturnValue == ERROR) {
        perror("Wait in main");
        return ERROR;
    }
    int munmapReturnValue = munmap(sharedMem, PAGE_SIZE);
    if (munmapReturnValue == ERROR){
        perror("munmap in main");
        return ERROR;
    }
    return SUCCESS;
}
