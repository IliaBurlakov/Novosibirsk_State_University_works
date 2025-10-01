#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define SUCCESS 0
#define ERROR 1
#define SLEEP_TIME 3
#define MESSAGE_INTERVAL 1
void *mythread()
{
    while (true)
    {
        printf("mythread [tid: %d]: running...\n", gettid());
        sleep(MESSAGE_INTERVAL);
    }
    return NULL;
}

int main()
{
    pthread_t tid;
    int err;
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    err = pthread_create(&tid, NULL, mythread, NULL);
    if (err != SUCCESS)
    {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return ERROR;
    }

    printf("main: thread was created. Sleeping for %d seconds...\n", SLEEP_TIME);
    sleep(SLEEP_TIME);

    err = pthread_cancel(tid);
    if (err != SUCCESS)
    {
        printf("main: pthread_cancel() failed: %s\n", strerror(err));
        return ERROR;
    }
    printf("main: thread was cancelled. Sleeping for %d seconds...\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
    return SUCCESS;
}
