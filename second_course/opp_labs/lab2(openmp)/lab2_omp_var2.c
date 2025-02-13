#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define PI 3.14159265358979323846
#define N 1600

void multiplyScalarAndVector(double *vector, double scalar, double *resVector) {
    #pragma omp for schedule(static)
    for (int i = 0; i < N; ++i)
        resVector[i] = vector[i] * scalar;
}

void subtractVectors(double *vector1, double *vector2, double *resVector) {
    #pragma omp for schedule(static)
    for (int i = 0; i < N; ++i)
        resVector[i] = vector1[i] - vector2[i];
}

void multiplyMatrixAndVector(double *matrix, double *vector, double *resVector) {
    #pragma omp for schedule(static)
    for (int i = 0; i < N; ++i) {
        double sum = 0;
        for (int j = 0; j < N; ++j)
            sum += matrix[i * N + j] * vector[j];
        resVector[i] = sum;
    }
}

double calculateNorm(double *vector) {
    double sum = 0;

    #pragma omp for
    for (int i = 0; i < N; ++i) {
        #pragma omp atomic
        sum += vector[i] * vector[i];
    }

    return sqrt(sum);
}

int check(double *vector1, double *vector2, double eps) {
    return (calculateNorm(vector1) / calculateNorm(vector2) < eps);
}

int main() {
    omp_set_num_threads(12);
    double *x = malloc(N * sizeof(double));
    double *b = malloc(N * sizeof(double));
    double *A = malloc(N * N * sizeof(double));
    double *Ax = malloc(N * sizeof(double));
    double *u = malloc(N * sizeof(double));
    double *tempVector = malloc(N * sizeof(double));

    const double epsilon = 1e-5;
    const double tao = 1e-3;

    for (int i = 0; i < N; ++i)
    {
        u[i] = sin(2 * PI * i / N);
        x[i] = 0;
        for (int j = 0; j < N; ++j)
            A[i * N + j] = (i == j) ? 2.0 : 1.0;
    }

    multiplyMatrixAndVector(A, u, b);
    double startTime = omp_get_wtime();

    #pragma omp parallel
    {
        while (1) {
            multiplyMatrixAndVector(A, x, Ax);
            subtractVectors(Ax, b, tempVector);
            if (check(tempVector, b, epsilon)) 
                break;
            multiplyScalarAndVector(tempVector, tao, tempVector);
            subtractVectors(x, tempVector, x);
        }
    }

    double endTime = omp_get_wtime();
    printf("%lf sec passed\n", endTime - startTime);
    free(x); 
    free(b); 
    free(A); 
    free(Ax); 
    free(u); 
    free(tempVector);
    return 0;
}