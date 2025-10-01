#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#define ERROR 1
#define SUCCESS 0
#define NUM_THREADS 5
#define NOT_EQUAL 0
#define THREAD_OFFSET 1

int global_var = 4;

void *mythread(void *arg)
{
    pid_t tid_from_gettid = gettid();
    pthread_t tid_from_create = *(pthread_t *)arg;
    pthread_t tid_from_self = pthread_self();
    int pid = getpid();
    int ppid = getppid();
    printf("mythread [pid %d, ppid %d]: Hello from mythread!\n", pid, ppid);
    printf("mythread tid's: gettid(): %d, pthread_self(): %lu, pthread_create(): %lu\n", tid_from_gettid, tid_from_self, tid_from_create);

    if (pthread_equal(tid_from_create, tid_from_self) != NOT_EQUAL)
        printf("Tid from pthread_create() is EQUAL to tid from pthread_self()!\n");
    if (pthread_equal(tid_from_create, tid_from_self) == NOT_EQUAL)
        printf("Tid from pthread_create() is NOT EQUAL to tid from pthread_self()!\n");

    if (pthread_equal(tid_from_create, tid_from_gettid) != NOT_EQUAL)
        printf("Tid from pthread_create() is EQUAL to tid from gettid()!\n");
    if (pthread_equal(tid_from_create, tid_from_gettid) == NOT_EQUAL)
        printf("Tid from pthread_create() is NOT EQUAL to tid from gettid()!\n");

    int local_var = 1;
    static int static_var = 2;
    const int const_var = 3;

    printf("Address of local_var: %p\n", (void *)&local_var);
    printf("Address of static_var: %p\n", (void *)&static_var);
    printf("Address of const_var: %p\n", (void *)&const_var);
    printf("Address of global_var: %p\n\n", (void *)&global_var);

    if (tid_from_gettid == pid + THREAD_OFFSET)
    {
        local_var = 666;
        global_var = 999;
        printf("Local and global variables were changed in thread with tid = %d. New values: local_var = 666, global_var = 999.\n\n", tid_from_gettid);
    }
    printf("TID: %d; local_var = %d, global_var = %d\n", tid_from_gettid, local_var, global_var);
    return NULL;
}

int main()
{
    pthread_t tid[NUM_THREADS];
    pthread_t *ptr_to_tids = tid;
    int err;
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
    for (int i = 0; i < NUM_THREADS; i++)
    {
        err = pthread_create(&tid[i], NULL, mythread, (void *)&ptr_to_tids[i]);
        if (err != SUCCESS)
        {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return ERROR;
        }
    }

    sleep(30);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        err = pthread_join(tid[i], NULL);
        if (err != SUCCESS)
        {
            printf("main: pthread_join() failed: %s\n", strerror(err));
            return ERROR;
        }
    }
    return SUCCESS;
}
