#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define PI 3.14159265358979323846
#define N 1600

void multiplyScalarAndVector(double *vector, double scalar, double *resVector)
{
    #pragma omp parallel for default(none) shared(vector, scalar, resVector) schedule(static)
    for (int i = 0; i < N; ++i)
        resVector[i] = vector[i] * scalar;
}

void addVectors(double *vector1, double *vector2, double *resVector)
{
    #pragma omp parallel for default(none) shared(vector1, vector2, resVector) schedule(static)
    for (int i = 0; i < N; ++i)
        resVector[i] = vector1[i] + vector2[i];
}

void subtractVectors(double *vector1, double *vector2, double *resVector)
{
#pragma omp parallel for default(none) shared(vector1, vector2, resVector) schedule(static)
    for (int i = 0; i < N; ++i)
        resVector[i] = vector1[i] - vector2[i];
}

void multiplyMatrixAndVector(double *matrix, double *vector, double *resVector)
{
#pragma omp parallel for default(none) shared(matrix, vector, resVector) schedule(static)
    for (int i = 0; i < N; ++i)
    {
        double sum = 0;
        for (int j = 0; j < N; ++j)
        {
            sum += matrix[i * N + j] * vector[j];
        }
        resVector[i] = sum;
    }
}

double calculateNorm(double *vector)
{
    double sum = 0;
#pragma omp parallel for reduction(+:sum) default(none) shared(vector) schedule(static)
    for (int i = 0; i < N; ++i)
    {
        sum += vector[i] * vector[i];
    }
    sum = sqrt(sum);
    return sum;
}

int check(double *vector1, double *vector2, double eps)
{
    return ((calculateNorm(vector1) / calculateNorm(vector2)) < eps);
}

int main()
{
    omp_set_num_threads(8);
    double *x = (double *)malloc(N * sizeof(double));
    double *b = (double *)malloc(N * sizeof(double));
    double *A = (double *)malloc(N * N * sizeof(double));
    double *Ax = (double *)malloc(N * sizeof(double));
    double *u = (double *)malloc(N * sizeof(double));
    double *tempVector = (double *)malloc(N * sizeof(double));

    const double epsilon = 1e-5;
    const double tao = 1e-3;

    for (int i = 0; i < N; ++i)
    {
        u[i] = sin(2 * PI * i / N);
        x[i] = 0;
        for (int j = 0; j < N; ++j)
        {
            if (i == j)
                A[i * N + j] = 2.0;
            else
                A[i * N + j] = 1.0;
        }
    }

    multiplyMatrixAndVector(A, u, b);

    double startTime, endTime;
    startTime = omp_get_wtime();

    while (1)
    {
        multiplyMatrixAndVector(A, x, Ax);
        subtractVectors(Ax, b, tempVector);
        if (check(tempVector, b, epsilon))
            break;
        multiplyScalarAndVector(tempVector, tao, tempVector);
        subtractVectors(x, tempVector, x);
    }
    endTime = omp_get_wtime();

    printf("%lf sec passed.\n", endTime - startTime);

    free(x);
    free(b);
    free(A);
    free(Ax);
    free(u);
    free(tempVector);

    return 0;
}