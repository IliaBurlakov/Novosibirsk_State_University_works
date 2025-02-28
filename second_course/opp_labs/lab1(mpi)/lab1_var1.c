#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846
#define N 1600

void multiplyScalarAndVector(double *vector, double scalar, double *resVector) {
    for (int i = 0; i < N; ++i) {
        resVector[i] = vector[i] * scalar;
    }
}

void addVectors(double *vector1, double *vector2, double *resVector) {
    for (int i = 0; i < N; ++i) {
        resVector[i] = vector1[i] + vector2[i];
    }
}

void subtractVectors(double *vector1, double *vector2, double *resVector) {
    for (int i = 0; i < N; ++i) {
        resVector[i] = vector1[i] - vector2[i];
    }
}

void multiplyMatrixAndVector(double *A_local, int myLocalN, double *x, double *resVector, int *counts, int *displs, MPI_Comm comm) {
    double *Ax_local = (double*)malloc(myLocalN * sizeof(double));
    for (int i = 0; i < myLocalN; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += A_local[i * N + j] * x[j];
        }
        Ax_local[i] = sum;
    }
    MPI_Allgatherv(Ax_local, myLocalN, MPI_DOUBLE, resVector, counts, displs, MPI_DOUBLE, comm);
    free(Ax_local);
}

double calculateNorm(double *vector, int myLocalN, MPI_Comm comm) {
    double localSum = 0.0;
    for (int i = 0; i < myLocalN; ++i) {
        localSum += vector[i] * vector[i];
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
    double *x = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *Ax = (double*)malloc(N * sizeof(double));
    double *tempVector = (double*)malloc(N * sizeof(double));
    const double epsilon = 1e-5;
    const double tao = 1e-3;
    double *u = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; ++i) {
        u[i] = sin(2 * PI * i / N);
        x[i] = 0.0;
    }
    
    for (int i = 0; i < myLocalN; ++i) {
        int globalRow = startRowGlobalIndex + i;
        for (int j = 0; j < N; ++j) {
            if (globalRow == j) 
                A_local[i * N + j] = 2.0;
            else 
                A_local[i * N + j] = 1.0;
        }
    }
    
    int *counts = (int*)malloc(numberOfProcesses * sizeof(int));
    int *displs = (int*)malloc(numberOfProcesses * sizeof(int));
    int prefix = 0;
    for (int r = 0; r < numberOfProcesses; r++) {
        int localN_r = (r < remainder) ? (base + 1) : base;
        counts[r] = localN_r;
        displs[r] = prefix;
        prefix += localN_r;
    }
    
    multiplyMatrixAndVector(A_local, myLocalN, u, b, counts, displs, comm);
    double t_start = MPI_Wtime();
    
    while (1) {
        multiplyMatrixAndVector(A_local, myLocalN, x, Ax, counts, displs, comm);
        subtractVectors(Ax, b, tempVector);
        double normTemp = calculateNorm(tempVector, myLocalN, comm);
        double normB = calculateNorm(b, myLocalN, comm);
        if (normTemp / normB < epsilon) break;
        multiplyScalarAndVector(tempVector, tao, tempVector);
        subtractVectors(x, tempVector, x);
    }
    
    double t_end = MPI_Wtime();
    if (rank == 0) {
        printf("%lf sec passed.\n", t_end - t_start);
        double maxDifference = 0.0;
        for (int i = 0; i < N; i++) {
            double difference = fabs(u[i] - x[i]);
            if (difference > maxDifference) maxDifference = difference;
        }
        printf("max difference: %lf\n", maxDifference);
    }
    
    free(A_local);
    free(x);
    free(b);
    free(Ax);
    free(tempVector);
    free(u);
    free(counts);
    free(displs);
    
    MPI_Finalize();
    return 0;
}
