#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERROR -1
#define SUCCESS 0
#define ERROR_MESSAGE_LENGHT 128
#define FORK_RET_VAL_FOR_CHILD 0

int global_var = 11;

void childProcessTask(int *local_var) {
    pid_t child_pid = getpid();
    pid_t parent_pid = getppid();
    printf("Child PID: %d\n", child_pid);
    printf("Parent PID: %d\n", parent_pid);
    printf("Global variable address in child process: %p, value: %d\n", &global_var, global_var);
    printf("Local variable address in child process: %p, value: %d\n", local_var, *local_var);

    global_var = 33;
    *local_var = 44;

    printf("Updated global variable value in child process: %d\n", global_var);
    printf("Updated local variable value in child process: %d\n", *local_var);
    int fflushReturnValue = fflush(stdout);
    if (fflushReturnValue == EOF){
        perror("fflush() in childProcessTask");
        _exit(ERROR);
    }
    int exitStatus = 5;
    _exit(exitStatus);
}

int parentProcessTask(int *local_var_value) {
    sleep(30);
    int status;
    pid_t waitReturnValue = wait(&status);
    if (waitReturnValue == ERROR) {
        perror("Wait in parentProcessTask");
        return ERROR;
    }
    printf("Global variable value in parent process: %d\n", global_var);
    printf("Local variable value in parent process: %d\n", *local_var_value);

    int terminatedWithSignal = WIFSIGNALED(status);
    int terminatedWithExit = WIFEXITED(status);
    if (terminatedWithSignal) {
        int signal = WTERMSIG(status);
        printf("Child process terminated by signal: %d\n", signal);
    }
    if (terminatedWithExit) {
        int exitStatus = WEXITSTATUS(status);
        printf("Child process terminated normally with exit code: %d\n", exitStatus);
    }
    return SUCCESS;
}

int main() {
    int local_var = 22; 
    printf("Global variable address: %p\n", &global_var);
    printf("Local variable address: %p\n", &local_var);
    pid_t originalPid = getpid();
    printf("Original PID: %d\n", originalPid);
    printf("Original values: global: %d, local: %d\n", global_var, local_var);
    pid_t pidFromFork = fork();

    if (pidFromFork == ERROR) {
        char errorMessage[ERROR_MESSAGE_LENGHT];
        snprintf(errorMessage, sizeof(errorMessage), "Can't fork process with PID %d", originalPid);
        perror(errorMessage);
        return ERROR;
    }
    if (pidFromFork == FORK_RET_VAL_FOR_CHILD) {
        childProcessTask(&local_var);
        return SUCCESS;
    }
    
    int parentProcessReturnValue;
    parentProcessReturnValue = parentProcessTask(&local_var);
    return parentProcessReturnValue;
}
