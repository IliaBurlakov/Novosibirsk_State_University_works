#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846
#define N 1600

void multiplyMatrixAndVector(double *A_local, int myLocalN, double *x_global, double *Ax_local)
{
    for (int i = 0; i < myLocalN; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += A_local[i * N + j] * x_global[j];
        }
        Ax_local[i] = sum;
    }
}

void subtractVectors(double *v1_local, double *v2_local, double *res_local, int myLocalN)
{
    for (int i = 0; i < myLocalN; ++i) {
        res_local[i] = v1_local[i] - v2_local[i];
    }
}

void multiplyScalarAndVector(double *v_local, double scalar, int myLocalN)
{
    for (int i = 0; i < myLocalN; ++i) {
        v_local[i] = v_local[i] * scalar;
    }
}

double calculateNorm(double *v_local, int myLocalN, MPI_Comm comm)
{
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
    double *r_local = (double*)malloc(myLocalN * sizeof(double));
    double *x_global = (double*)malloc(N * sizeof(double)); // to calculate A_local * x_global.

    const double epsilon = 1e-5;
    const double tao = 1e-3;

    for (int i = 0; i < myLocalN; ++i) {
        int globalRow = startRowGlobalIndex + i;
        for (int j = 0; j < N; ++j) {
            if (globalRow == j)
                A_local[i*N + j] = 2.0;
            else
                A_local[i*N + j] = 1.0;
        }
    }

    for (int i = 0; i < myLocalN; ++i) {
        x_local[i] = 0.0;
    }

    for (int i = 0; i < myLocalN; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; j++) {
            double u_j = sin(2.0 * PI * j / N);
            sum += A_local[i*N + j] * u_j;
        }
        b_local[i] = sum;
    }

    int *counts  = (int*)malloc(numberOfProcesses*sizeof(int));
    int *displs = (int*)malloc(numberOfProcesses*sizeof(int));
    int prefix = 0;
    for (int r = 0; r < numberOfProcesses; r++) {
        int localN_r = (r < remainder) ? (base + 1) : base;
        counts[r] = localN_r;
        displs[r] = prefix;
        prefix += localN_r;
    }

    double t_start = MPI_Wtime();

    while (1) {
        MPI_Allgatherv(x_local, myLocalN, MPI_DOUBLE, x_global, counts, displs, MPI_DOUBLE, comm);
        multiplyMatrixAndVector(A_local, myLocalN, x_global, Ax_local);
        subtractVectors(Ax_local, b_local, r_local, myLocalN); // r_local = Ax_local - b_local

        double norm_r = calculateNorm(r_local, myLocalN, comm);
        double norm_b = calculateNorm(b_local, myLocalN, comm);

        if (norm_r / norm_b < epsilon) {
            break;
        }
        multiplyScalarAndVector(r_local, tao, myLocalN);
        for (int i = 0; i < myLocalN; ++i) {
            x_local[i] = x_local[i] - r_local[i];
        }
    }

    double t_end = MPI_Wtime();

    if (rank == 0) {
        printf("%lf sec passed.\n", t_end - t_start);
    }
    free(A_local);
    free(x_local);
    free(b_local);
    free(Ax_local);
    free(r_local);
    free(x_global);
    free(counts);
    free(displs);
    MPI_Finalize();
    return 0;
}
