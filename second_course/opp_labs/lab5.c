    #include <stdio.h>
    #include <stddef.h>
    #include <string.h>
    #include <pthread.h>
    #include <mpi.h>
    #include <stdlib.h>
    #include <math.h>
    #include <time.h>
    #include <stdatomic.h>

    #define N_TASKS 128
    #define MAX_DIFFICULTY_LEVEL 10 
    #define REQUEST_TAG 0
    #define TASK_TAG 1
    #define ITERATIONS 5

    typedef struct {
        int task_id;
        int difficulty;
        double result;
        int wasTransferred;
    } Task;

    typedef struct {
        Task* tasks;
        int count;
        pthread_mutex_t mutex;
    } TaskQueue;

    typedef struct {
        int rank;
        int initial_weight;
        int completed_tasks;
        int total_weight;
        int own_tasks;
        double work_time;
    } ProcessInfo;

    typedef struct {
        int rank;
        int size;
        ProcessInfo* process_info;
        MPI_Datatype mpi_task_type;
        MPI_Datatype mpi_process_type;
        double local_result; 
        pthread_mutex_t result_mutex; 
    } ThreadArgs;

    TaskQueue local_queue;



    MPI_Datatype create_task_type() {
        MPI_Datatype task_type;
        int block_lengths[4] = {1, 1, 1, 1};
        MPI_Aint displacements[4];
        MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_DOUBLE, MPI_INT};
    
        displacements[0] = offsetof(Task, task_id);
        displacements[1] = offsetof(Task, difficulty);
        displacements[2] = offsetof(Task, result);
        displacements[3] = offsetof(Task, wasTransferred);
    
        MPI_Type_create_struct(4, block_lengths, displacements, types, &task_type);
        MPI_Type_commit(&task_type);
    
        return task_type;
    }
    
    MPI_Datatype create_process_info_type() {
        MPI_Datatype process_info_type;
        int block_lengths[6] = {1, 1, 1, 1, 1, 1};
        MPI_Aint displacements[6];
        MPI_Datatype types[6] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE};
    
        displacements[0] = offsetof(ProcessInfo, rank);
        displacements[1] = offsetof(ProcessInfo, initial_weight);
        displacements[2] = offsetof(ProcessInfo, completed_tasks);
        displacements[3] = offsetof(ProcessInfo, total_weight);
        displacements[4] = offsetof(ProcessInfo, own_tasks);
        displacements[5] = offsetof(ProcessInfo, work_time);
    
        MPI_Type_create_struct(6, block_lengths, displacements, types, &process_info_type);
        MPI_Type_commit(&process_info_type);
    
        return process_info_type;
    }

    void add_task(TaskQueue* q, Task task) {
        pthread_mutex_lock(&q->mutex);
        if (q->count < N_TASKS) {
            q->tasks[q->count++] = task;
        } else {
            fprintf(stderr, "Queue overflow!\n");
        }
        pthread_mutex_unlock(&q->mutex);
    }

    int get_task(TaskQueue* q, Task* task) {
        pthread_mutex_lock(&q->mutex);
        if(q->count == 0) {
            pthread_mutex_unlock(&q->mutex);
            return 0;
        }
        *task = q->tasks[--q->count];
        pthread_mutex_unlock(&q->mutex);
        return 1;
    }

    double complex_calculation(int difficulty) {
        double result = 0.0;
        double x = sqrt(difficulty);
        for(int i = 0; i < difficulty * 10e6; i++) {
            result += sqrt(fabs(sin(x) * cos(x)));
        }
        return result;
    }

    void* worker_thread(void* args) {
        ThreadArgs* targs = (ThreadArgs*)args;
        int rank = targs->rank;
        int size = targs->size;
        ProcessInfo* process_info = targs->process_info;
        MPI_Datatype MPI_TASK_TYPE = targs->mpi_task_type;
        MPI_Datatype MPI_PROCESS_INFO_TYPE = targs->mpi_process_type;
        Task task;
        int ownTasksCount = 0;
        int initial_weight = 0;  
        int completed_tasks = 0; 
        int total_weight = 0;  

        for (int i = 0; i < local_queue.count; i++) {
            initial_weight += local_queue.tasks[i].difficulty;
        }
        double timeOfCalculations = 0.0;
        double startTimeOfCalculations;
        double endTimeOfCalculations;
        
        while (1) {
            if (get_task(&local_queue, &task)) {
                startTimeOfCalculations = MPI_Wtime();
                task.result = complex_calculation(task.difficulty);
                endTimeOfCalculations = MPI_Wtime();
                timeOfCalculations += endTimeOfCalculations - startTimeOfCalculations;
                pthread_mutex_lock(&targs->result_mutex);
                targs->local_result += task.result;
                pthread_mutex_unlock(&targs->result_mutex);

                completed_tasks++;
                total_weight += task.difficulty;
                if (task.wasTransferred == 0)
                    ownTasksCount++;
                continue;
            }
            if (targs->size > 1) { 
            int left = (rank - 1 + size) % size;
            int right = (rank + 1) % size;
            MPI_Request req[2];
            MPI_Isend(&rank, 1, MPI_INT, left, REQUEST_TAG, MPI_COMM_WORLD, &req[0]);
            MPI_Isend(&rank, 1, MPI_INT, right, REQUEST_TAG, MPI_COMM_WORLD, &req[1]);

            MPI_Wait(&req[0], MPI_STATUS_IGNORE);
            MPI_Wait(&req[1], MPI_STATUS_IGNORE);
            Task received_task;
            int received = 0;
            MPI_Status status;
            for (int i = 0; i < 2; i++) {
                MPI_Recv(&received_task, 1, MPI_TASK_TYPE, MPI_ANY_SOURCE, TASK_TAG, MPI_COMM_WORLD, &status);
                if (received_task.task_id != -1) {
                    add_task(&local_queue, received_task);
                    received = 1;
                    break;
                }
            }
            if (received) continue;

            for (int i = 0; i < size; i++) {
                if (i == rank || i == left || i == right) continue;
                MPI_Send(&rank, 1, MPI_INT, i, REQUEST_TAG, MPI_COMM_WORLD);
                MPI_Recv(&received_task, 1, MPI_TASK_TYPE, i, TASK_TAG, MPI_COMM_WORLD, &status);
                if (received_task.task_id != -1) {
                    add_task(&local_queue, received_task);
                    received = 1;
                    break;
                }
            }
            if (received) continue;
        }
            pthread_mutex_lock(&local_queue.mutex);
            int local_count = local_queue.count;    
            pthread_mutex_unlock(&local_queue.mutex);
            if (targs->size == 1) {
                if (local_count == 0) break;
                else continue;
            }
            int global_count;
            MPI_Allreduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            if (global_count == 0) break;
        }

        process_info->rank = rank;
        process_info->initial_weight = initial_weight;
        process_info->completed_tasks = completed_tasks;
        process_info->total_weight = total_weight;
        process_info->own_tasks =  ownTasksCount;
        process_info->work_time = timeOfCalculations;

        int stop_signal = -1;
        if (size > 1)
            MPI_Send(&stop_signal, 1, MPI_INT, rank, REQUEST_TAG, MPI_COMM_WORLD);
        return NULL;
    }
    void* server_thread(void* args) {
        ThreadArgs* targs = (ThreadArgs*)args;
        int rank = targs->rank;
        MPI_Datatype MPI_TASK_TYPE = targs->mpi_task_type;
        MPI_Status status;
    
        while (1) {
            int request;
            MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST_TAG, MPI_COMM_WORLD, &status);
    
            if (status.MPI_SOURCE == rank && request == -1) break;
    
            Task task_to_send;
            if (get_task(&local_queue, &task_to_send)) {
                task_to_send.wasTransferred = 1;    
                MPI_Send(&task_to_send, 1, MPI_TASK_TYPE, status.MPI_SOURCE, TASK_TAG, MPI_COMM_WORLD);
            } else {
                Task empty_task = {.task_id = -1};
                MPI_Send(&empty_task, 1, MPI_TASK_TYPE, status.MPI_SOURCE, TASK_TAG, MPI_COMM_WORLD);
            }
        }
        return NULL;
    }


    void hillDistribution(int* numberOfTasksForProcesses, int size, int reversed) {
        int totalTasks = N_TASKS;
        int totalAssignedTasks = 0;
        
        for (int i = 0; i < size; i++) {
            int index = reversed ? (size - 1 - i) : i;
            numberOfTasksForProcesses[index] = (i * i) + 1;
            totalAssignedTasks += numberOfTasksForProcesses[index];
        }

        double normalization = (double) totalTasks / totalAssignedTasks;
        
        for (int i = 0; i < size; i++) {
            numberOfTasksForProcesses[i] = (int)(numberOfTasksForProcesses[i] * normalization);
        }

        int remaining_tasks = totalTasks;
        for (int i = 0; i < size; i++) {
            remaining_tasks -= numberOfTasksForProcesses[i];
        }

        for (int i = 0; remaining_tasks > 0; i = (i + 1) % size) {
            numberOfTasksForProcesses[i]++;
            remaining_tasks--;
        }
    }

    void uniformDistribution(int* numberOfTasksForProcesses, int size){
        int perProcess = N_TASKS / size;
        for(int i = 0; i < size; i++){
            numberOfTasksForProcesses[i] = perProcess;
        }
    }

    void degenerateDistribution(int* numberOfTasksForProcesses, int size){
        numberOfTasksForProcesses[0] = N_TASKS;
        for (int i = 1; i < size; i++){
            numberOfTasksForProcesses[i] = 0;
        }
    }

    void evenOddDistribution(int* numberOfTasksForProcesses, int size, int evenDistribution) {
        memset(numberOfTasksForProcesses, 0, size * sizeof(int));
        
        int evenProcessesNumber = (size % 2 == 0) ? (size / 2) : (size / 2 + 1);
        int oddProcessesNumber = size - evenProcessesNumber;
        
        int perProcessTasks = evenDistribution ? N_TASKS / evenProcessesNumber : N_TASKS / oddProcessesNumber;
        
        for(int i = 0; i < size; i++) {
            if (evenDistribution && (i % 2 == 0)) {
                numberOfTasksForProcesses[i] = perProcessTasks;
            } else if (!evenDistribution && (i % 2 != 0)) {
                numberOfTasksForProcesses[i] = perProcessTasks;
            }
        }
        
        int remainingTasks = N_TASKS - (evenDistribution ? evenProcessesNumber : oddProcessesNumber) * perProcessTasks;
        
        for (int i = 0; remainingTasks > 0; i = (i + 1) % size) {
            if ((evenDistribution && (i % 2 == 0)) || (!evenDistribution && (i % 2 != 0))) {
                numberOfTasksForProcesses[i]++;
                remainingTasks--;
            }
        }
    }

    void distributeTasks(int distributionCode, int* numberOfTasksForProcesses, int size){
        if (size == 1) {
            degenerateDistribution(numberOfTasksForProcesses, size);
            return;
        }
        switch (distributionCode){
            case 0: uniformDistribution(numberOfTasksForProcesses, size); break;
            case 1: hillDistribution(numberOfTasksForProcesses, size, 0); break;
            case 2: hillDistribution(numberOfTasksForProcesses, size, 1); break;
            case 3: degenerateDistribution(numberOfTasksForProcesses, size); break;
            case 4: evenOddDistribution(numberOfTasksForProcesses, size, 0); break;
            case 5: evenOddDistribution(numberOfTasksForProcesses, size, 1); break;
        }
    }

    int main(int argc, char** argv) {
        if (argc != 2){
            printf("Usage: ./%s <distribution code>. Distributions:\n0 - uniform, 1 - hill, 2 - reversed hill, 3 - degenerate, 4 - only odd, 5 - only even\n", argv[0]);
            return 0;
        }
        int distributionCode = atoi(argv[1]);
        if (distributionCode < 0 || distributionCode > 5){
            printf("Distribution code incorrect.\n");
            return 0;
        }

        int rank, size;
        int provided;
        double start_time, end_time;
        MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
        if (provided != MPI_THREAD_MULTIPLE){
            printf("Required thread support level is not MPI_THREAD_MULTIPLE. Finishing the program.\n");
            printf("Provided level: %d\n", provided);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Datatype MPI_TASK_TYPE = create_task_type();
        MPI_Datatype MPI_PROCESS_INFO_TYPE = create_process_info_type();
        pthread_mutex_init(&local_queue.mutex, NULL);


        int* counts = malloc(size * sizeof(int));
        int* displacements = malloc(size * sizeof(int));
        if (!counts || !displacements) {
            fprintf(stderr, "Memory allocation failed for counts or displacements\n");
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        TaskQueue global_queue; 
        if(rank == 0) {
            global_queue.tasks = malloc(N_TASKS * sizeof(Task));
            if (!global_queue.tasks) {
                fprintf(stderr, "Memory allocation failed for global_queue.tasks\n");
                MPI_Abort(MPI_COMM_WORLD, 3);
            }
            global_queue.count = 0;

            int tasksComplexity[N_TASKS];
            for (int i = 0; i < N_TASKS; i++){
                tasksComplexity[i] = (i * i * i) % 10 + 1;
                Task t = {
                    .task_id = i,
                    .difficulty = tasksComplexity[i],
                    .wasTransferred = 0
                };
                add_task(&global_queue, t);
            }

            int* numberOfTasksForProcesses = malloc(size * sizeof(int));
            if (!numberOfTasksForProcesses) {
                fprintf(stderr, "Memory allocation failed for numberOfTasksForProcesses\n");
                MPI_Abort(MPI_COMM_WORLD, 2);
            }
            distributeTasks(distributionCode, numberOfTasksForProcesses, size);
            displacements[0] = 0;
            counts[0] = numberOfTasksForProcesses[0];
            for(int i = 1; i < size; i++) {
                counts[i] = numberOfTasksForProcesses[i];
                displacements[i] = displacements[i-1] + counts[i-1];
            }
            free(numberOfTasksForProcesses);
        }
        MPI_Bcast(counts, size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(displacements, size, MPI_INT, 0, MPI_COMM_WORLD);

        local_queue.tasks = malloc((counts[rank] > 0 ? counts[rank] : 1) * sizeof(Task));
        if (!local_queue.tasks) {
            fprintf(stderr, "Memory allocation failed for local_queue.tasks\n");
            MPI_Abort(MPI_COMM_WORLD, 4);
        }
        ProcessInfo* global_process_infos = NULL;
        ProcessInfo* iteration_process_infos = NULL;
        ProcessInfo* current_process_info = malloc(sizeof(ProcessInfo));
        if (current_process_info == NULL){
            fprintf(stderr, "Memory allocation failed for current_process_info.tasks\n");
            MPI_Abort(MPI_COMM_WORLD, 5);
        }
        if(rank == 0) {
            global_process_infos = calloc(size, sizeof(ProcessInfo));
            iteration_process_infos = malloc(size * sizeof(ProcessInfo));
        }
        
        ThreadArgs thread_args = {
            .rank = rank,
            .size = size,
            .process_info = current_process_info,
            .mpi_task_type = MPI_TASK_TYPE,
            .mpi_process_type = MPI_PROCESS_INFO_TYPE,
            .local_result = 0.0,                  
            .result_mutex = PTHREAD_MUTEX_INITIALIZER 
        };
        double final_result = 0.0;
        Task* original_tasks = NULL;
        if (rank == 0) {
            original_tasks = malloc(N_TASKS * sizeof(Task));
            memcpy(original_tasks, global_queue.tasks, N_TASKS * sizeof(Task));
        }  
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        for (int j = 0; j < ITERATIONS; j++){
            local_queue.count = counts[rank]; 
            memset(current_process_info, 0, sizeof(ProcessInfo));
            thread_args.local_result = 0.0;
            if (rank == 0) 
                memcpy(global_queue.tasks, original_tasks, N_TASKS * sizeof(Task));
            MPI_Scatterv(global_queue.tasks, counts, displacements, MPI_TASK_TYPE, local_queue.tasks, counts[rank], MPI_TASK_TYPE, 0, MPI_COMM_WORLD);

        
            pthread_t worker;
            pthread_create(&worker, NULL, worker_thread, &thread_args);
            if (size > 1)
                server_thread(&thread_args);
        
            pthread_join(worker, NULL);

            double global_result = 0.0;
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Reduce(&thread_args.local_result, &global_result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Gather(
                current_process_info,
                1,                          
                MPI_PROCESS_INFO_TYPE,     
                (rank == 0) ? iteration_process_infos : NULL,
                (rank == 0) ? 1 : 0,        
                MPI_PROCESS_INFO_TYPE,    
                0,             
                MPI_COMM_WORLD
            ); 
            if (rank == 0){
                for(int i = 0; i < size; i++) {
                    final_result += global_result;
                    global_process_infos[i].rank = iteration_process_infos[i].rank;
                    global_process_infos[i].initial_weight += iteration_process_infos[i].initial_weight;
                    global_process_infos[i].total_weight += iteration_process_infos[i].total_weight;
                    global_process_infos[i].completed_tasks += iteration_process_infos[i].completed_tasks;
                    global_process_infos[i].own_tasks += iteration_process_infos[i].own_tasks;
                    global_process_infos[i].work_time += iteration_process_infos[i].work_time;
                }
                
            }        
        }
        end_time = MPI_Wtime();
        if(rank == 0) {
            printf("Final result: %.5f\n", final_result);
            printf("Time taken: %.3f\n", end_time - start_time);
            double maxProcessTime = 0;
            double minProcessTime = end_time - start_time;
            for (int i = 0; i < size; i++){
                if (global_process_infos[i].work_time > maxProcessTime)
                    maxProcessTime = global_process_infos[i].work_time;
                if (global_process_infos[i].work_time < minProcessTime)
                    minProcessTime = global_process_infos[i].work_time;
            }
            double loadImbalance = (double) maxProcessTime / minProcessTime;
            printf("Load imbalance: %.3f\n\n\n", loadImbalance);
            for(int i = 0; i < size; i++) {
                printf("-------------------------------------------------\n");
                printf("Process %d:\n", global_process_infos[i].rank);
                printf("Initial total weight: %d\n", global_process_infos[i].initial_weight);
                printf("Total weight of completed tasks: %d\n", global_process_infos[i].total_weight);
                printf("Completed tasks: %d\n", global_process_infos[i].completed_tasks);
                double ownTasksPercentage = global_process_infos[i].completed_tasks == 0 ? 0 : ((double) global_process_infos[i].own_tasks / global_process_infos[i].completed_tasks * 100);
                printf("Percentage of own tasks: %.2f%%\n", ownTasksPercentage);
                printf("Useful working time: %.3f sec\n", global_process_infos[i].work_time);
                printf("-------------------------------------------------\n\n\n");
            }
            free(global_process_infos);
            free(iteration_process_infos);
            free(original_tasks);
            free(global_queue.tasks);
        }
        free(current_process_info);
        free(counts);
        free(displacements);
        free(local_queue.tasks);
        pthread_mutex_destroy(&local_queue.mutex);
        MPI_Type_free(&MPI_TASK_TYPE);
        MPI_Type_free(&MPI_PROCESS_INFO_TYPE);
        MPI_Finalize();
        return 0;
    }
