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
#define SLEEP_TIME 1

struct mydata
{
    int number;
    char *message;
};

void *mythread(void *arg)
{
    struct mydata *data = (struct mydata *)arg;

    printf("mythread [tid: %d]: number = %d, message = %s\n", gettid(), data->number, data->message);
    free(data);
    return NULL;
}

int init_detached_attr(pthread_attr_t *attr)
{
    int err;
    err = pthread_attr_init(attr);
    if (err != SUCCESS)
    {
        printf("pthread_attr_init() failed: %s\n", strerror(err));
        return ERROR;
    }
    err = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
    if (err != SUCCESS)
    {
        printf("pthread_attr_setdetachstate() failed: %s\n", strerror(err));
        pthread_attr_destroy(attr);
        return ERROR;
    }
    return SUCCESS;
}

int main()
{
    pthread_t tid;
    pthread_attr_t attr;
    int err;
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
    struct mydata *data = malloc(sizeof(struct mydata));
    if (data == NULL)
    {
        printf("main: malloc() failed\n");
        return ERROR;
    }
    data->number = 123;
    data->message = "hello world detached";
    int init_attr_return_val = init_detached_attr(&attr);
    if (init_attr_return_val == ERROR)
    {
        free(data);
        return ERROR;
    }
    err = pthread_create(&tid, &attr, mythread, data);
    if (err != SUCCESS)
    {
        fprintf(stderr, "pthread_create() failed: %s\n", strerror(err));
        pthread_attr_destroy(&attr);
        free(data);
        return ERROR;
    }
    pthread_attr_destroy(&attr);
    printf("main: sleeping for %d second...\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
    return SUCCESS;
}
