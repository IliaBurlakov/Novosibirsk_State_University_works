#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define SUCCESS 0
#define ERROR 1
#define SLEEP_TIME 5
#define SIZEOF_NULL_TERMINATOR 1
#define MESSAGE_INTERVAL 1
#define EXECUTE_HANDLER 1

void cleanup_free(void *arg)
{
    char *ptr = (char *)arg;
    if (ptr != NULL) {
        free(ptr);
        printf("cleanup: memory freed\n");
    }
}

void *mythread()
{
    char *hello = "hello world";
    char *message = malloc(strlen(hello) + SIZEOF_NULL_TERMINATOR);
    if (message == NULL)
    {
        printf("mythread: malloc() failed\n");
        return NULL;
    }
    strcpy(message, hello);
    pthread_cleanup_push(cleanup_free, message);
    int counter = 0;
    while (true) {
        printf("mythread [tid: %d]: %s\n", gettid(), message);
        counter++;
        if (counter == 2)
        {
            pthread_exit(NULL);
        }
        sleep(MESSAGE_INTERVAL);
    }

    pthread_cleanup_pop(EXECUTE_HANDLER);
    return NULL;
}

int main(void)
{
    pthread_t tid;
    int err;
    void *thread_result;

    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    err = pthread_create(&tid, NULL, mythread, NULL);
    if (err != SUCCESS)
    {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        return ERROR;
    }
    printf("main: thread created. Sleeping for %d seconds...\n", SLEEP_TIME);
    sleep(SLEEP_TIME);

    err = pthread_cancel(tid);
    if (err != SUCCESS)
    {
        printf("main: pthread_cancel() failed: %s\n", strerror(err));
        return ERROR;
    }

    err = pthread_join(tid, &thread_result);
    if (err != SUCCESS)
    {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return ERROR;
    }
    if (thread_result == PTHREAD_CANCELED)
    {
        printf("main: thread was canceled\n");
    }
    return SUCCESS;
}
