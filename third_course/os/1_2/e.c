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
    int err = pthread_detach(pthread_self());
    if (err != SUCCESS)
    {
        printf("pthread_detach error");
        return NULL;
    }
    printf("mythread [tid: %d]: Hello from mythread!\n", gettid());
    return NULL;
}

int main()
{
    pthread_t tid;
    int err;
    int threads_num = 0;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    while (true)
    {
        err = pthread_create(&tid, NULL, mythread, NULL);
        if (err != SUCCESS)
        {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            break;
        }
        threads_num++;
    }

    printf("main [%d %d %d]: Exiting after %d threads\n", getpid(), getppid(), gettid(), threads_num);
    return SUCCESS;
}