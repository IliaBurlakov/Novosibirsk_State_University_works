#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define SUCCESS 0
#define ERROR 1

void *mythread()
{
    printf("mythread [tid: %d]: Hello from mythread!\n", gettid());
    return NULL;
}

int main()
{
    pthread_t tid;
    int err;
    int threads_num = 0;
    pthread_attr_t attr;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    err = pthread_attr_init(&attr);
    if (err != SUCCESS)
    {
        printf("main: pthread_attr_init() failed: %s\n", strerror(err));
        return ERROR;
    }
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err != SUCCESS)
    {
        printf("main: pthread_attr_setdetachstate() failed: %s\n", strerror(err));
        pthread_attr_destroy(&attr);
        return ERROR;
    }
    while (true)
    {
        err = pthread_create(&tid, &attr, mythread, NULL);
        if (err != SUCCESS)
        {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            break;
        }
        threads_num++;
    }
    err = pthread_attr_destroy(&attr);
    if (err != SUCCESS)
    {
        printf("main: pthread_attr_destroy() failed: %s\n", strerror(err));
    }
    printf("main [%d %d %d]: Exiting after %d threads\n", getpid(), getppid(), gettid(), threads_num);
    return SUCCESS;
}