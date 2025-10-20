#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define SUCCESS 0
#define ERROR -1
#define SLEEP_TIME 100
#define SIZEOF_NULL_TERMINATOR 1

void *thread_block_all()
{
    int err;
    sigset_t all;
    sigfillset(&all);
    err = pthread_sigmask(SIG_BLOCK, &all, NULL);
    if (err != SUCCESS) {
        printf("thread_block_all: pthread_sigmask failed: %s\n", strerror(err));
        return (void *)ERROR;
    }
    printf("thread_block_all [pid %d tid %d]: blocked all signals. Sleeping...\n", getpid(), gettid());
    while(true)
        sleep(SLEEP_TIME);
    return NULL;
}

void my_sigint_handler(int signum)
{
    if (signum != SIGINT) {
        return;
    }
    const char msg[] = "sigint handler: received SIGINT\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - SIZEOF_NULL_TERMINATOR);
}

void *thread_with_handler()
{
    int err;
    sigset_t unblock_sigint;
    sigemptyset(&unblock_sigint);
    err = sigaddset(&unblock_sigint, SIGINT);
    if (err != SUCCESS) {
        printf("thread_with_handler: sigaddset failed: %s\n", strerror(errno));
        return (void *)ERROR;
    }
    err = pthread_sigmask(SIG_UNBLOCK, &unblock_sigint, NULL);
    if (err != SUCCESS) {
        printf("thread_with_handler: pthread_sigmask failed: %s\n", strerror(err));
        return (void *)ERROR;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = my_sigint_handler;
    err = sigaction(SIGINT, &sa, NULL);
    if (err != SUCCESS) {
        printf("thread_with_handler: sigaction failed: %s\n", strerror(errno));
        return (void *)ERROR;
    }

    printf("thread_with_handler [pid %d tid %d]: installed handler for SIGINT and unblocked SIGINT. Sleeping...\n", getpid(), gettid());
    while (true)
        sleep(SLEEP_TIME);
    return NULL;
}

void *thread_sigwait()
{
    int err;
    sigset_t waitset;
    int sig = 0;

    sigemptyset(&waitset);
    sigaddset(&waitset, SIGQUIT);

    printf("thread_sigwait [pid %d tid %d]: waiting for SIGQUIT\n", getpid(), gettid());
    err = sigwait(&waitset, &sig);
    if (err != SUCCESS) {
        printf("thread_sigwait: sigwait failed: %s\n", strerror(errno));
        return (void *)ERROR;
    }
    printf("thread_sigwait [pid %d tid %d]: sigwait received signal %d\n", getpid(), gettid(), sig);
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;
    int err;
    sigset_t mask;
    printf("main [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());
    sigemptyset(&mask);
    err = sigaddset(&mask, SIGINT);
    if (err != SUCCESS) {
        printf("main: sigaddset failed: %s\n", strerror(errno));
        return ERROR;
    }
    err = sigaddset(&mask, SIGQUIT);
    if (err != SUCCESS) {
        printf("main: sigaddset failed: %s\n", strerror(errno));
        return ERROR;
    }
    err = pthread_sigmask(SIG_BLOCK, &mask, NULL);
    if (err != SUCCESS) {
        printf("main: pthread_sigmask failed: %s\n", strerror(err));
        return ERROR;
    }
    err = pthread_create(&t1, NULL, thread_block_all, NULL);
    if (err != SUCCESS){
        printf("main: pthread_create(t1) failed: %s\n", strerror(err));
        return ERROR;
    }
    err = pthread_create(&t2, NULL, thread_with_handler, NULL);
    if (err != SUCCESS){
        printf("main: pthread_create(t2) failed: %s\n", strerror(err));
        return ERROR;
    }
    err = pthread_create(&t3, NULL, thread_sigwait, NULL);
    if (err != SUCCESS){
        printf("main: pthread_create(t3) failed: %s\n", strerror(err));
        return ERROR;
    }

    printf("main: threads created. Commands to send signals:\nkill -SIGINT %d \nkill -SIGQUIT %d\n", getpid(), getpid());

    sleep(SLEEP_TIME);
    return SUCCESS;
}
