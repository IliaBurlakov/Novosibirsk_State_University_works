#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define eps 1e-8
#define a 1e5

#define X0 -1
#define Y0 -1
#define Z0 -1

#define Dx 2.0
#define Dy 2.0
#define Dz 2.0

#define Nx 400
#define Ny 400
#define Nz 400

#define hx (Dx / (Nx - 1.0))
#define hy (Dy / (Ny - 1.0))
#define hz (Dz / (Nz - 1.0))

double phi(double x, double y, double z) {
    return x * x + y * y + z * z;
}

double ro(double x, double y, double z) {
    return 6 - a * phi(x, y, z);
}

double doJacobiMethodCenter(int rank, int layerHeight, double* previousPhi, int x, int y, int z, double multiplier) {
    double xComp = (previousPhi[Nx * Ny * z + Nx * y + (x - 1)] + previousPhi[Nx * Ny * z + Nx * y + (x + 1)]) / (hx * hx);
    double yComp = (previousPhi[Nx * Ny * z + Nx * (y - 1) + x] + previousPhi[Nx * Ny * z + Nx * (y + 1) + x]) / (hy * hy);
    double zComp = (previousPhi[Nx * Ny * (z - 1) + Nx * y + x] + previousPhi[Nx * Ny * (z + 1) + Nx * y + x]) / (hz * hz);
    return multiplier * (xComp + yComp + zComp - ro(X0 + x * hx, Y0 + y * hy, Z0 + (z + layerHeight * rank) * hz));
}

double doJacobiMethodTopEdge(int rank, int layerHeight, double* previousPhi, int x, int y, const double* upLayer, double multiplier) {
    double xComp = (previousPhi[Nx * Ny * (layerHeight - 1) + Nx * y + (x - 1)] + previousPhi[Nx * Ny * (layerHeight - 1) + Nx * y + (x + 1)]) / (hx * hx);
    double yComp = (previousPhi[Nx * Ny * (layerHeight - 1) + Nx * (y - 1) + x] + previousPhi[Nx * Ny * (layerHeight - 1) + Nx * (y + 1) + x]) / (hy * hy);
    double zComp = (previousPhi[Nx * Ny * (layerHeight - 2) + Nx * y + x] + upLayer[Nx * y + x]) / (hz * hz);
    return multiplier * (xComp + yComp + zComp - ro(X0 + x * hx, Y0 + y * hy, Z0 + ((layerHeight - 1) + layerHeight * rank) * hz));                
}

double doJacobiMethodBottomEdge(int rank, int layerHeight, double* previousPhi, int x, int y, const double* downLayer, double multiplier) {
    double xComp = (previousPhi[Nx * y + (x - 1)] + previousPhi[Nx * y + (x + 1)]) / (hx * hx);
    double yComp = (previousPhi[Nx * (y - 1) + x] + previousPhi[Nx * (y + 1) + x]) / (hy * hy);
    double zComp = (downLayer[Nx * y + x] + previousPhi[Nx * Ny + Nx * y + x]) / (hz * hz);
    return multiplier * (xComp + yComp + zComp - ro(X0 + x * hx, Y0 + y * hy, Z0 + (layerHeight * rank) * hz));
}

void calculateCenter(int layerHeight, double* previousPhi, double* Phi, int rank, char* flag, double multiplier) {
    for (int z = 1; z < layerHeight - 1; ++z) {
        for (int y = 1; y < Ny - 1; ++y) {
            for (int x = 1; x < Nx - 1; ++x) {
                Phi[Nx * Ny * z + Nx * y + x] = doJacobiMethodCenter(rank, layerHeight, previousPhi, x, y, z, multiplier);
                if (fabs(Phi[Nx * Ny * z + Nx * y + x] - previousPhi[Nx * Ny * z + Nx * y + x]) > eps) { 
                    (*flag) = 0;
                }
            }
        }
    }
}

void calculateEdges(int layerHeight, double* previousPhi, double* Phi, int rank, char* flag, const double* downLayer, const double* upLayer, int size, double multiplier) {
    for (int y = 1; y < Ny - 1; ++y) {
        for (int x = 1; x < Nx - 1; ++x) {
            if (rank != 0) {
                Phi[Nx * y + x] = doJacobiMethodBottomEdge(rank, layerHeight, previousPhi, x, y, downLayer, multiplier);
                if (fabs(Phi[Nx * y + x] - previousPhi[Nx * y + x]) > eps) {
                    (*flag) = 0;
                }
            }
            if (rank != size - 1) {
                int index = Nx * Ny * (layerHeight - 1) + Nx * y + x;
                Phi[index] = doJacobiMethodTopEdge(rank, layerHeight, previousPhi, x, y, upLayer, multiplier);
                if (fabs(Phi[index] - previousPhi[index]) > eps) {
                    (*flag) = 0;
                }
            }
        }
    }
}

double calculateMaxDifference(int rank, int layerHeight, double* Phi) { 
    double max = 0; 
    double diff = 0; 
    for (int z = 0; z < layerHeight; ++z) { 
        for (int y = 0; y < Ny; ++y) {
            for (int x = 0; x < Nx; ++x) { 
                diff = fabs(Phi[z * Nx * Ny + y * Nx + x] - phi(X0 + x * hx, Y0 + y * hy, Z0 + (z + layerHeight * rank) * hz)); 
                if (diff > max) {
                    max = diff; 
                }
            }
        }
    }
    double tmp = 0; 
    MPI_Allreduce(&max, &tmp, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD); // MPI_ALLREDUCE(sendbuf, recvbuf, count, datatype, op, comm)
    max = tmp;
    return max;
}

int main(int argc, char** argv) {
    int rank, size;
    double timeStart, timeFinish;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int layerHeight = Nz / size;
    double* Phi = (double*)malloc(sizeof(double) * Nx * Ny * layerHeight);
    double* previousPhi = (double*)malloc(sizeof(double) * Nx * Ny * layerHeight);
    double* downLayer = (double*)malloc(sizeof(double) * Nx * Ny);
    double* upLayer = (double*)malloc(sizeof(double) * Nx * Ny);
    double multiplier = 1.0 / (2.0 / (hx * hx) + 2.0 / (hy * hy) + 2.0 / (hz * hz) + a);
    if (rank == 0) timeStart = MPI_Wtime();

    for (int z = 0; z < layerHeight; ++z) {
        for (int y = 0; y < Ny; ++y) {
            for (int x = 0; x < Nx; ++x) {
                if (y == 0 || x == 0 || y == Ny - 1 || x == Nx - 1) {
                    Phi[Nx * Ny * z + Nx * y + x] = phi(X0 + x * hx, Y0 + y * hy, Z0 + (z + layerHeight * rank) * hz);
                    previousPhi[Nx * Ny * z + Nx * y + x] = phi(X0 + x * hx, Y0 + y * hy, Z0 + (z + layerHeight * rank) * hz);
                }
                else {
                    Phi[Nx * Ny * z + Nx * y + x] = 0;
                    previousPhi[Nx * Ny * z + Nx * y + x] = 0;
                }
            }
        }
    }
    if (rank == 0) {
        for (int y = 0; y < Ny; ++y) {
            for (int x = 0; x < Nx; ++x) {
                Phi[Nx * y + x] = phi(X0 + x * hx, Y0 + y * hy, Z0);
                previousPhi[0 + Nx * y + x] = phi(X0 + x * hx, Y0 + y * hy, Z0);
            }
        }
    }

    if (rank == size - 1) {
        for (int y = 0; y < Ny; ++y) {
            for (int x = 0; x < Nx; ++x) {
                Phi[Nx * Ny * (layerHeight - 1) + Nx * y + x] = phi(X0 + x * hx, Y0 + y * hy, Z0 + Dz);
                previousPhi[Nx * Ny * (layerHeight - 1) + Nx * y + x] = phi(X0 + x * hx, Y0 + y * hy, Z0 + Dz);
            }
        }
    }
	
    double* tmp;
    int counter = 0;
    MPI_Request requests[4];

    char isEnoughIterations = 1;
    do {
        isEnoughIterations = 1;
        tmp = previousPhi;
        previousPhi = Phi;
        Phi = tmp;

        if (rank != 0) {
            MPI_Isend(&previousPhi[0], Nx * Ny, MPI_DOUBLE, rank - 1, 10, MPI_COMM_WORLD, &requests[0]); // MPI_ISEND(buf, count, datatype, dest, tag, comm, request)
            MPI_Irecv(downLayer, Nx * Ny, MPI_DOUBLE, rank - 1, 20, MPI_COMM_WORLD, &requests[1]); // MPI_IRECV(buf, count, datatype, source, tag, comm, request)
        }

        if (rank != size - 1) {
            MPI_Isend(&previousPhi[(layerHeight - 1) * Nx * Ny], Nx * Ny, MPI_DOUBLE, rank + 1, 20, MPI_COMM_WORLD, &requests[2]);
            MPI_Irecv(upLayer, Nx * Ny, MPI_DOUBLE, rank + 1, 10, MPI_COMM_WORLD, &requests[3]);
        }

        calculateCenter(layerHeight, previousPhi, Phi, rank, &isEnoughIterations, multiplier);

        if (rank != 0) {
            MPI_Wait(&requests[0], MPI_STATUS_IGNORE);
            MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
        }
        if (rank != size - 1) {
            MPI_Wait(&requests[2], MPI_STATUS_IGNORE);
            MPI_Wait(&requests[3], MPI_STATUS_IGNORE);
        }

        calculateEdges(layerHeight, previousPhi, Phi, rank, &isEnoughIterations, downLayer, upLayer, size, multiplier);

        char tmpFlag;
        MPI_Allreduce(&isEnoughIterations, &tmpFlag, 1, MPI_CHAR, MPI_LAND, MPI_COMM_WORLD);
        isEnoughIterations = tmpFlag;

        if (rank == 0) counter++;
    } while (!isEnoughIterations);

    if (rank == 0) timeFinish = MPI_Wtime();
    double max = calculateMaxDifference(rank, layerHeight, Phi); 
    if (rank == 0) {
        printf("Number of iterations: %d\n", counter);
        printf("Time: %lf sec.\n", (timeFinish - timeStart));
        printf("Max difference: %lf\n", max);
    }

    MPI_Finalize();
    return 0;
}
