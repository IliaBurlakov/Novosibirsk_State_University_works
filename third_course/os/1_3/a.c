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

struct myStruct
{
    int number;
    char *message;
};

void *mythread(void *arg)
{
    struct myStruct *data = (struct myStruct *)arg;
    printf("mythread [tid: %d]: number = %d, message = %s\n", gettid(), data->number, data->message);
    return NULL;
}

int main()
{
    pthread_t tid;
    int err;
    struct myStruct data;
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    data.number = 123;
    data.message = "hello world";

    err = pthread_create(&tid, NULL, mythread, &data);
    if (err != SUCCESS)
    {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return ERROR;
    }

    err = pthread_join(tid, NULL);
    if (err != SUCCESS)
    {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return ERROR;
    }

    return SUCCESS;
}
