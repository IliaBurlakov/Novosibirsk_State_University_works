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
    char *hello = "hello world";
    printf("String to return from mythread: %s\n", hello);
    return (void *)hello;
}

int main()
{
    pthread_t tid;
    int err;
    char *thread_message;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    err = pthread_create(&tid, NULL, mythread, NULL);
    if (err != SUCCESS)
    {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return ERROR;
    }

    err = pthread_join(tid, (void **)&thread_message);
    if (err != SUCCESS)
    {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return ERROR;
    }

    printf("Received string in main: %s\n", thread_message);
    return SUCCESS;
}