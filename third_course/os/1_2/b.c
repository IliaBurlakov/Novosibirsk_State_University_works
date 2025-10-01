#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR 1

void *mythread()
{
    printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
    int *valueToReturn = malloc(sizeof(int));
    if (valueToReturn == NULL)
    {
        printf("mythread: malloc error");
        return NULL;
    }

    *valueToReturn = 42;
    printf("Value to return from mythread: %d\n", *valueToReturn);
    return (void *)valueToReturn;
}

int main()
{
    pthread_t tid;
    int err;
    int *thread_result;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    err = pthread_create(&tid, NULL, mythread, NULL);
    if (err != SUCCESS)
    {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return ERROR;
    }

    err = pthread_join(tid, (void **)&thread_result);
    if (err != SUCCESS)
    {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return ERROR;
    }

    printf("Received value in main: %d\n", *thread_result);

    free(thread_result);
    return SUCCESS;
}