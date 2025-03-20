#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846
#define N 1600

    void multiplyMatrixAndVector(double *A_local, int myLocalN, double *x_local, double *Ax_local, int *counts, int *displs, int numberOfProcesses, int rank, MPI_Comm comm) {
        for (int i = 0; i < myLocalN; i++) {
            Ax_local[i] = 0.0;
        }
        int maxLocalN = ((N % numberOfProcesses) == 0) ? (N / numberOfProcesses) :  (N /numberOfProcesses + 1);
        double *sendBuf = (double*)malloc(maxLocalN * sizeof(double));
        double *recvBuf = (double*)malloc(maxLocalN * sizeof(double));
        int currentSize = counts[rank];
        memcpy(sendBuf, x_local, currentSize * sizeof(double));
        int currentOwner = rank;
        for (int step = 0; step < numberOfProcesses; step++) {
            int blockSize = counts[currentOwner];
            int blockDisp = displs[currentOwner];
            for (int i = 0; i < myLocalN; i++) {
                double part = 0.0;
                for (int k = 0; k < blockSize; k++) {
                    int globalIndex = blockDisp + k;
                    part += A_local[i*N + globalIndex] * sendBuf[k];
                }
                Ax_local[i] += part;
            }
            int leftNeighbour = (rank - 1 + numberOfProcesses) % numberOfProcesses;
            int rightNeighbour = (rank + 1) % numberOfProcesses;
            int sendCurSizeAndCurOwner[2] = { currentSize, currentOwner };
            int recvPrevSizeAndPrevOwner[2];
            MPI_Sendrecv(sendCurSizeAndCurOwner, 2, MPI_INT, rightNeighbour, 0, recvPrevSizeAndPrevOwner, 2, MPI_INT, leftNeighbour, 0, comm, MPI_STATUS_IGNORE); // ( void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status )
            int nextSize = recvPrevSizeAndPrevOwner[0];
            int nextOwner = recvPrevSizeAndPrevOwner[1];
            MPI_Sendrecv(sendBuf, currentSize, MPI_DOUBLE, rightNeighbour, 1, recvBuf, nextSize, MPI_DOUBLE, leftNeighbour, 1, comm, MPI_STATUS_IGNORE);
            currentSize = nextSize;
            currentOwner = nextOwner;
            memcpy(sendBuf, recvBuf, currentSize * sizeof(double));
        }
        free(sendBuf);
        free(recvBuf);
    }

double calculateNorm(double *v_local, int myLocalN, MPI_Comm comm) {
    double localSum = 0.0;
    for (int i = 0; i < myLocalN; ++i) {
        localSum += v_local[i] * v_local[i];
    }
    double globalSum = 0.0;
    MPI_Allreduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, comm);
    return sqrt(globalSum);
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    int numberOfProcesses, rank;
    MPI_Comm_size(comm, &numberOfProcesses);
    MPI_Comm_rank(comm, &rank);
    int base = N / numberOfProcesses;
    int remainder = N % numberOfProcesses;
    int myLocalN = (rank < remainder) ? (base + 1) : base;
    int startRowGlobalIndex = 0;
    for (int r = 0; r < rank; r++) {
        startRowGlobalIndex += (r < remainder) ? (base + 1) : base;
    }

    double *A_local = (double*)malloc(myLocalN * N * sizeof(double));
    double *x_local = (double*)malloc(myLocalN * sizeof(double));
    double *b_local = (double*)malloc(myLocalN * sizeof(double));
    double *Ax_local = (double*)malloc(myLocalN * sizeof(double));
    double *temp_local = (double*)malloc(myLocalN * sizeof(double));

    int *counts = (int*)malloc(numberOfProcesses * sizeof(int));
    int *displs = (int*)malloc(numberOfProcesses * sizeof(int));
    int prefix = 0;
    for (int i = 0; i < numberOfProcesses; i++) {
        int localN_i = (i < remainder) ? (base + 1) : base;
        counts[i] = localN_i;
        displs[i] = prefix; 
        prefix += counts[i];
    }

    for (int i = 0; i < myLocalN; i++) {
        int globalRow = startRowGlobalIndex + i;
        for (int j = 0; j < N; j++) {
            if (globalRow == j) 
                A_local[i * N + j] = 2.0;
            else 
                A_local[i * N + j] = 1.0;
        }
        x_local[i] = 0.0;
    }

    double* u = (double*)malloc(N * sizeof(double));
    for (int i = 0; i < N; i++){
        u[i] = sin(2.0 * PI * i / N);
    }
    for (int i = 0; i < myLocalN; i++) {
        double sum = 0.0;
        for (int j = 0; j < N; j++) {
            sum += A_local[i*N + j] * u[j];
        }
        b_local[i] = sum;
    }
    const double epsilon = 1e-2;
    const double tao = 1e-3;
    double t_start = MPI_Wtime();
    while (1) {
        multiplyMatrixAndVector(A_local, myLocalN, x_local, Ax_local, counts, displs, numberOfProcesses, rank, comm);
        for (int i = 0; i < myLocalN; i++) {
            temp_local[i] = Ax_local[i] - b_local[i];
        }
        double normTemp = calculateNorm(temp_local, myLocalN, comm);
        double normB = calculateNorm(b_local, myLocalN, comm);
        if (normTemp / normB < epsilon) {
            break;
        }
        for (int i = 0; i < myLocalN; i++) {
            x_local[i] = x_local[i] - tao * temp_local[i];
        }
    }
    double t_end = MPI_Wtime();
    double *x_global = (double*)malloc(N * sizeof(double));
    MPI_Allgatherv(x_local, myLocalN, MPI_DOUBLE, x_global, counts, displs, MPI_DOUBLE, comm);
    if (rank == 0) {
        printf("%lf sec passed.\n", t_end - t_start);
        double maxDifference = 0;
        for (int i = 0; i < N; i++){
            double difference = fabs(u[i] - x_global[i]);
            if (difference > maxDifference)
                maxDifference = difference;
        }
        printf("max difference: %lf\n", maxDifference);

    }
    free(A_local);
    free(x_local);
    free(b_local);
    free(Ax_local);
    free(temp_local);
    free(counts);
    free(displs);
    free(u);
    MPI_Finalize();
    return 0;
}
