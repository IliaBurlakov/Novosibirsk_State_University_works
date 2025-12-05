#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h> 

#define BUFFER_SIZE 8192
#define MAX_RETRIES 100
#define RETRY_DELAY 1
#define THREAD_POOL_SIZE 128
#define QUEUE_SIZE 10000
#define SUCCESS 0
#define ERROR -1
#define EXPECTED_ARGC 3
#define NULL_TERM_SIZE 1
#define STR_EQUAL 0
#define OPEN_MODE_UNUSED 0

#define JOIN_THREAD(thread) do { \
    int err = pthread_join(thread, NULL); \
    if (err != SUCCESS) \
        printf("JOIN_THREAD: pthread_join failed: %s\n", strerror(err)); \
} while(0)

#define DESTROY_MUTEX(mutex) do { \
    int err = pthread_mutex_destroy(mutex); \
    if (err != SUCCESS) \
        printf("DESTROY_MUTEX: pthread_mutex_destroy failed: %s\n", strerror(err)); \
} while(0)

#define DESTROY_COND(cond) do { \
    int err = pthread_cond_destroy(cond); \
    if (err != SUCCESS) \
        printf("DESTROY_COND: pthread_cond_destroy failed: %s\n", strerror(err)); \
} while(0)

#define SAFE_CLOSE(fd) do { \
    int err = close(fd); \
    if (err != SUCCESS) { \
        printf("SAFE_CLOSE: close failed for fd %d: %s\n", (fd), strerror(errno)); \
    } \
} while(0)

typedef enum {
    TASK_COPY_FILE,
    TASK_PROCESS_DIR
} task_type_t;

typedef struct {
    task_type_t type;
    char source_path[PATH_MAX];
    char target_path[PATH_MAX];
} task_t;

typedef struct {
    pthread_t* threads;
    task_t* task_queue;
    int queue_size;
    int read_slot_idx;
    int write_slot_idx;
    int active_tasks;
    int shutdown;
    
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
} thread_pool_t;

int check_inside(const char* parent, const char* child) {
    if (strcmp(parent, child) == STR_EQUAL) {
        printf("check_inside: target equals source\n");
        return ERROR;
    }
    size_t par_len = strlen(parent);
    if (strncmp(parent, child, par_len) == STR_EQUAL && (child[par_len] == '/')) {
        printf("check_inside: target '%s' is inside source '%s'\n", child, parent);
        return ERROR;
    }
    return SUCCESS;
}

int build_final_dst(const char* dst, char* dst_final, size_t size) {
    char tmp_parent[PATH_MAX];
    char tmp_base[PATH_MAX];
    strncpy(tmp_parent, dst, PATH_MAX);
    tmp_parent[PATH_MAX - NULL_TERM_SIZE] = '\0';
    strncpy(tmp_base, dst, PATH_MAX);
    tmp_base[PATH_MAX - NULL_TERM_SIZE] = '\0';

    char* parent = dirname(tmp_parent);
    char* base = basename(tmp_base);

    char* parent_real = realpath(parent, NULL);
    if (parent_real == NULL) {
        printf("build_final_dst: error resolving parent of '%s': %s\n", dst, strerror(errno));
        return ERROR;
    }
    int printed_count = snprintf(dst_final, size, "%s/%s", parent_real, base);
    if (printed_count >= (int)size) {
        printf("build_final_dst: resulting path too long\n");
        free(parent_real);
        return ERROR;
    }

    free(parent_real);
    return SUCCESS;
}

int is_target_inside_source(const char* src, const char* dst) {
    int err;
    char* src_real = realpath(src, NULL);
    if (src_real == NULL) {
        printf("is_target_inside_source: error resolving source '%s': %s\n", src, strerror(errno));
        return ERROR;
    }
    char* dst_real = realpath(dst, NULL);

    if (dst_real != NULL) {
        err = check_inside(src_real, dst_real);
        free(src_real);
        free(dst_real);
        return err;
    }
    char dst_final[PATH_MAX];
    err = build_final_dst(dst, dst_final, sizeof(dst_final));
    if (err == ERROR) {
        free(src_real);
        return ERROR;
    }
    err = check_inside(src_real, dst_final);
    free(src_real);
    return err;
}

int open_file_with_retry(const char* path, int flags, mode_t mode) {
    int fd;
    int retries = 0;
    
    while (true) {
        fd = open(path, flags, mode);
        if (fd != ERROR) {
            break;
        }
        if (errno != EMFILE) {
            printf("open_file_with_retry: open failed for %s: %s\n", path, strerror(errno));
            break;
        }
        if (retries > MAX_RETRIES) {
            printf("open_file_with_retry: max retries reached for %s\n", path);
            break;
        }
        retries++;
        sleep(RETRY_DELAY);
    }
    
    return fd;
}

DIR* open_dir_with_retry(const char* path) {
    DIR* dir = NULL;
    int retries = 0;

    while (true) {
        dir = opendir(path);
        if (dir != NULL) {
            break;
        }
        if (errno != EMFILE) {
            printf("open_dir_with_retry: error opening directory '%s': %s\n", path, strerror(errno));
            break;
        }
        if (retries >= MAX_RETRIES) {
            printf("open_dir_with_retry: max retries reached for '%s'\n", path);
            break;
        }

        retries++;
        sleep(RETRY_DELAY);
    }

    return dir;
}

int build_safe_path(char* dest, size_t dest_size, const char* dir, const char* file) {
    size_t dir_len = strlen(dir);
    size_t file_len = strlen(file);
    if (dir_len + file_len + NULL_TERM_SIZE > dest_size) {
        printf("build_safe_path: resulting path too long: %s/%s\n", dir, file);
        return ERROR;
    }
    snprintf(dest, dest_size, "%s/%s", dir, file);
    return SUCCESS;
}

void destroy_thread_pool(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        JOIN_THREAD(pool->threads[i]);
    }
    DESTROY_MUTEX(&pool->queue_mutex);
    DESTROY_COND(&pool->queue_cond);
    free(pool->task_queue);
    free(pool->threads);
}

int add_task(thread_pool_t* pool, task_type_t type, const char* source_path, const char* target_path) {
    int err;
    err = pthread_mutex_lock(&pool->queue_mutex);
    if (err != SUCCESS) {
        printf("add_task: pthread_mutex_lock failed: %s\n", strerror(err));
        return ERROR;
    }
    if (pool->shutdown == true) {
        err = pthread_mutex_unlock(&pool->queue_mutex);
        if (err != SUCCESS) {
            printf("add_task: pthread_mutex_unlock failed: %s\n", strerror(err));
        }
        return ERROR;
    }
    
    while ((pool->queue_size == QUEUE_SIZE) && (pool->shutdown == false)) {
        pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
    }
    
    if (pool->shutdown == true) {
        err = pthread_mutex_unlock(&pool->queue_mutex);
        if (err != SUCCESS) {
            printf("add_task: pthread_mutex_unlock failed: %s\n", strerror(err));
        }
        return ERROR;
    }
    
    task_t* task = &pool->task_queue[pool->write_slot_idx];
    task->type = type;
    strncpy(task->source_path, source_path, PATH_MAX);
    strncpy(task->target_path, target_path, PATH_MAX);
    task->source_path[PATH_MAX - NULL_TERM_SIZE] = '\0';
    task->target_path[PATH_MAX - NULL_TERM_SIZE] = '\0';
    
    pool->write_slot_idx = (pool->write_slot_idx + 1) % QUEUE_SIZE;
    pool->queue_size++;
    pool->active_tasks++;
    
    pthread_cond_broadcast(&pool->queue_cond);
    err = pthread_mutex_unlock(&pool->queue_mutex);
    if (err != SUCCESS) {
        printf("add_task: pthread_mutex_unlock failed: %s\n", strerror(err));
        return ERROR;
    }
    return SUCCESS;
}

void wait_for_all_tasks(thread_pool_t* pool) {
    int err;
    err = pthread_mutex_lock(&pool->queue_mutex);
    if (err != SUCCESS) {
        printf("wait_for_all_tasks: pthread_mutex_lock failed: %s\n", strerror(err));
        return;
    }
    while (pool->active_tasks > 0) {
        pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
    }
    
    err = pthread_mutex_unlock(&pool->queue_mutex);
    if (err != SUCCESS) {
        printf("wait_for_all_tasks: pthread_mutex_unlock failed: %s\n", strerror(err));
    }
}

int copy_file(const char* source_path, const char* target_path) {
    int source_fd;
    int target_fd;
    ssize_t bytes_read;
    ssize_t bytes_written;
    char buffer[BUFFER_SIZE];
    struct stat stat_buf;
    int err;
    
    err = lstat(source_path, &stat_buf);
    if (err != SUCCESS) {
        printf("copy_file: error stating file %s: %s\n", source_path, strerror(errno));
        return ERROR;
    }
    source_fd = open_file_with_retry(source_path, O_RDONLY, OPEN_MODE_UNUSED);
    if (source_fd == ERROR) {
        printf("copy_file: error opening source file %s: %s\n", source_path, strerror(errno));
        return ERROR;
    }
    target_fd = open_file_with_retry(target_path, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode);
    if (target_fd == ERROR) {
        printf("copy_file: error creating target file %s: %s\n", target_path, strerror(errno));
        SAFE_CLOSE(source_fd);
        return ERROR;
    }
    
    while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
        char* ptr = buffer;
        ssize_t remaining = bytes_read;
        while (remaining > 0) {
            bytes_written = write(target_fd, ptr, remaining);
            if (bytes_written == ERROR) {
                printf("copy_file: error writing to file %s: %s\n", target_path, strerror(errno));
                SAFE_CLOSE(source_fd);
                SAFE_CLOSE(target_fd);
                return ERROR;
            }
            remaining -= bytes_written;
            ptr += bytes_written;
        }
    }
    if (bytes_read == ERROR) {
        printf("copy_file: error reading from file %s: %s\n", source_path, strerror(errno));
        SAFE_CLOSE(source_fd);
        SAFE_CLOSE(target_fd);
        return ERROR;
    }
    SAFE_CLOSE(source_fd);
    SAFE_CLOSE(target_fd);
    return SUCCESS;
}

int process_single_entry(thread_pool_t* pool, const char* source_dir, const char* target_dir, const char* entry_name) {
    char source_path[PATH_MAX];
    char target_path[PATH_MAX];
    struct stat stat_buf;
    int err;
    
    err = build_safe_path(source_path, PATH_MAX, source_dir, entry_name);
    if (err != SUCCESS) {
        printf("Path too long: %s/%s\n", source_dir, entry_name);
        return ERROR;
    }
    
    err = build_safe_path(target_path, PATH_MAX, target_dir, entry_name);
    if (err != SUCCESS) {
        printf("Path too long: %s/%s\n", target_dir, entry_name);
        return ERROR;
    }
    
    err = lstat(source_path, &stat_buf);
    if (err != SUCCESS) {
        printf("Error stating %s: %s\n", source_path, strerror(errno));
        return ERROR;
    }
    
    if (S_ISREG(stat_buf.st_mode)) {
        return add_task(pool, TASK_COPY_FILE, source_path, target_path);
    }
    if (S_ISDIR(stat_buf.st_mode)) {
        return add_task(pool, TASK_PROCESS_DIR, source_path, target_path);
    }
    
    return SUCCESS;
}

int process_directory(thread_pool_t* pool, const char* source_path, const char* target_path) {
    DIR* dir = NULL;
    struct dirent* entry = NULL;
    struct stat stat_buf;
    int err;
    
    err = lstat(source_path, &stat_buf);
    if (err != SUCCESS) {
        printf("process_directory: error stating directory %s: %s\n", source_path, strerror(errno));
        return ERROR;
    }
    err = mkdir(target_path, stat_buf.st_mode);
    if (err != SUCCESS && errno != EEXIST) {
        printf("process_directory: mkdir failed for %s: %s\n", target_path, strerror(errno));
        return ERROR;
    }
    dir = open_dir_with_retry(source_path);
    if (dir == NULL) {
        return ERROR;
    }

    while (true) {
        errno = SUCCESS;
        entry = readdir(dir);
        if (entry == NULL && errno != SUCCESS) {
            printf("process_directory: error reading directory %s: %s\n", source_path, strerror(errno));
            err = ERROR;
            break;
        }
        if (entry == NULL) {
            err = SUCCESS;
            break;
        }
        if (strcmp(entry->d_name, ".") == STR_EQUAL || strcmp(entry->d_name, "..") == STR_EQUAL) {
            continue;
        }
        err = process_single_entry(pool, source_path, target_path, entry->d_name);
        if (err != SUCCESS) {
            printf("process_directory: failed to add task for %s\n", entry->d_name);
        }
    }
    err = closedir(dir);
    if (err != SUCCESS) {
        printf("process_directory: closedir failed for %s: %s\n", source_path, strerror(errno));
    }
    return err;
}

void* worker_thread(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;
    int err;
    while (true) {
        err = pthread_mutex_lock(&pool->queue_mutex);
        if (err != SUCCESS) {
            printf("worker_thread: pthread_mutex_lock failed: %s\n", strerror(err));
            break;
        }
        while ((pool->queue_size == 0) && (pool->shutdown == false)) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        
        if (pool->shutdown == true) {
            err = pthread_mutex_unlock(&pool->queue_mutex);
            if (err != SUCCESS) {
                printf("worker_thread: pthread_mutex_unlock failed: %s\n", strerror(err));
            }
            break;
        }
        task_t task = pool->task_queue[pool->read_slot_idx];
        pool->read_slot_idx = (pool->read_slot_idx + 1) % QUEUE_SIZE;
        pool->queue_size--;
        pthread_cond_broadcast(&pool->queue_cond);
        err = pthread_mutex_unlock(&pool->queue_mutex);
        if (err != SUCCESS) {
            printf("worker_thread: pthread_mutex_unlock failed: %s\n", strerror(err));
            break;
        }
        if (task.type == TASK_COPY_FILE) {
            err = copy_file(task.source_path, task.target_path);
        }
        if (task.type == TASK_PROCESS_DIR) {
            err = process_directory(pool, task.source_path, task.target_path);
        }
        if (err != SUCCESS) {
            printf("worker_thread: failed to process task: %s -> %s\n", task.source_path, task.target_path);
        }
        err = pthread_mutex_lock(&pool->queue_mutex);
        if (err != SUCCESS) {
            printf("worker_thread: pthread_mutex_lock failed: %s\n", strerror(err));
            break;
        }
        pool->active_tasks--;
        if (pool->active_tasks == 0) {
            pthread_cond_broadcast(&pool->queue_cond);
        }
        err = pthread_mutex_unlock(&pool->queue_mutex);
        if (err != SUCCESS) {
            printf("worker_thread: pthread_mutex_unlock failed: %s\n", strerror(err));
            break;
        }
    }
    return NULL;
}

int init_thread_pool(thread_pool_t* pool) {
    int err;
    pool->task_queue = malloc(QUEUE_SIZE * sizeof(task_t));
    if (pool->task_queue == NULL) {
        printf("init_thread_pool: malloc for task queue failed\n");
        return ERROR;
    }
    pool->queue_size = 0;
    pool->read_slot_idx = 0;
    pool->write_slot_idx = 0;
    pool->active_tasks = 0;
    pool->shutdown = false;
    pool->threads = malloc(THREAD_POOL_SIZE * sizeof(pthread_t));
    if (pool->threads == NULL) {
        printf("init_thread_pool: malloc for threads failed\n");
        free(pool->task_queue);
        return ERROR;
    }
    err = pthread_mutex_init(&pool->queue_mutex, NULL);
    if (err != SUCCESS) {
        printf("init_thread_pool: mutex init failed: %s\n", strerror(err));
        free(pool->task_queue);
        free(pool->threads);
        return ERROR;
    }
    err = pthread_cond_init(&pool->queue_cond, NULL);
    if (err != SUCCESS) {
        printf("init_thread_pool: cond init failed: %s\n", strerror(err));
        DESTROY_MUTEX(&pool->queue_mutex);
        free(pool->task_queue);
        free(pool->threads);
        return ERROR;
    }
    
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        err = pthread_create(&pool->threads[i], NULL, worker_thread, (void*)pool);
        if (err != SUCCESS) {
            printf("init_thread_pool: thread create failed: %s\n", strerror(err));
            for (int j = 0; j < i; j++) {
                JOIN_THREAD(pool->threads[j]);
            }
            DESTROY_MUTEX(&pool->queue_mutex);
            DESTROY_COND(&pool->queue_cond);
            free(pool->task_queue);
            free(pool->threads);
            return ERROR;
        }
    }
    
    return SUCCESS;
}
int main(int argc, char* argv[]) {
    int err;
    struct stat stat_buf;
    thread_pool_t thread_pool;
    if (argc != EXPECTED_ARGC) {
        printf("Usage: %s <source_directory> <target_directory>\n", argv[0]);
        return ERROR;
    }
    err = lstat(argv[1], &stat_buf);
    if (err != SUCCESS) {
        printf("Error accessing source directory %s: %s\n", argv[1], strerror(errno));
        return ERROR;
    }
    int is_directory = S_ISDIR(stat_buf.st_mode);
    if (is_directory == false) {
        printf("Source path %s is not a directory\n", argv[1]);
        return ERROR;
    }
    err = is_target_inside_source(argv[1], argv[2]);
    if (err != SUCCESS) {
        return ERROR;
    }
    err = init_thread_pool(&thread_pool);
    if (err != SUCCESS) {
        return ERROR;
    }
    err = add_task(&thread_pool, TASK_PROCESS_DIR, argv[1], argv[2]);
    if (err != SUCCESS) {
        printf("Failed to add root task\n");
        destroy_thread_pool(&thread_pool);
        return ERROR;
    }
    wait_for_all_tasks(&thread_pool);
    destroy_thread_pool(&thread_pool);
    printf("main: all tasks were completed, thread pool was destroyed\n");
    return SUCCESS;
}