#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <iomanip>
#include <random>
/*#include <cmath>
#include <cblas.h>

double getMaxDiffWithCorrectAnswer(const std::vector<double>& A, const std::vector<double>& B, const std::vector<double>& C, int n1, int n2, int n3){
    std::vector<double> D(n1 * n3, 0.0);
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n1, n3, n2, 1.0, A.data(), n2, B.data(), n3, 0.0, D.data(), n3);
    double maxDiff = 0.0;
    for (size_t i = 0; i < D.size(); ++i) {
      double diff = std::abs(D[i] - C[i]);
      if (diff > maxDiff) {
        maxDiff = diff;
      }
    }
    return maxDiff;
  }
 */
  void fillMatrixRandom(std::vector<double>& matrix, int countElements) {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_real_distribution<double> dist(1, 100);
      for (int i = 0; i < countElements; ++i) {
        matrix[i] = dist(gen);
      }
  }

void PrintMatrix(const double* matrix, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        std::cout << matrix[i * cols + j] << " ";
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

void multiplySubmatrices(const std::vector<double>& subA, const std::vector<double>& subB, std::vector<double>& subC, int sub_n1, int n2, int sub_n3) {
    for (int row = 0; row < sub_n1; row++) {
        for (int k = 0; k < n2; k++) {
            double a = subA[row * n2 + k];
            for (int col = 0; col < sub_n3; col++) {
                subC[row * sub_n3 + col] += a * subB[k * sub_n3 + col];
            }
        }
    }  
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    double startTime = 0.0, endTime = 0.0;
    int dims[2] = {0, 0};
    int reorder = 0;
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (argc != 4 && rank == 0) {
        printf("Usage: %s <n1> <n2> <n3>\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int n1 = std::atoi(argv[1]);
    int n2 = std::atoi(argv[2]);
    int n3 = std::atoi(argv[3]);

    MPI_Dims_create(size, 2, dims); // MPI_DIMS_CREATE(nnodes, ndims, dims)
    if (rank == 0)
        std::cout << "Dimensions: " << dims[0] << " " << dims[1] << std::endl;

    if ((n1 % dims[0] != 0) || (n3 % dims[1] != 0)) {
        if (rank == 0)
            std::cout << "n1, n3 must be divisible by p1, p2!" << std::endl;
        MPI_Finalize();
        return 1;
    }

    MPI_Comm cartComm, columnComm, rowComm;
    int periods[2] = {0, 0};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartComm); // MPI_CART_CREATE(comm_old, ndims, dims, periods, reorder, comm_cart)
    int coords[2];
    MPI_Cart_coords(cartComm, rank, 2, coords); // MPI_CART_COORDS(comm, rank, maxdims, coords)

    int subDims[2];
    subDims[0] = 0; subDims[1] = 1;
    MPI_Cart_sub(cartComm, subDims, &rowComm); // MPI_CART_SUB(comm, remain_dims, newcomm)
    subDims[0] = 1; subDims[1] = 0;
    MPI_Cart_sub(cartComm, subDims, &columnComm);

    int sub_n1 = n1 / dims[0];
    int sub_n3 = n3 / dims[1];

    std::vector<double> subA(sub_n1 * n2);
    std::vector<double> subB(n2 * sub_n3);
    std::vector<double> subC(sub_n1 * sub_n3);
    std::vector<double> A, B, C;
    if (coords[0] == 0 && coords[1] == 0) {
        A.resize(n1 * n2);
        B.resize(n2 * n3);
        C.resize(n1 * n3);
        fillMatrixRandom(A, n1 * n2);
        fillMatrixRandom(B, n2 * n3);
        startTime = MPI_Wtime();
    }

    MPI_Datatype subtype_A;
    MPI_Type_contiguous(sub_n1 * n2, MPI_DOUBLE, &subtype_A); // MPI_TYPE_CONTIGUOUS (count, oldtype, newtype)
    MPI_Type_commit(&subtype_A);
    if (coords[1] == 0) {
        MPI_Scatter((coords[0]==0 ? A.data() : nullptr), 1, subtype_A, subA.data(), 1, subtype_A, 0, columnComm);
        // MPI_SCATTER(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm)
    }
    MPI_Bcast(subA.data(), sub_n1 * n2, MPI_DOUBLE, 0, rowComm); // MPI_BCAST(buffer, count, datatype, root, comm)
    MPI_Type_free(&subtype_A);

    MPI_Datatype subtype_B, subtype_B_tmp;
    MPI_Type_vector(n2, sub_n3, n3, MPI_DOUBLE, &subtype_B_tmp); // MPI_TYPE_VECTOR(count, blocklength, stride, oldtype, newtype)
    // stride - число элементов между началами каждого блока
    MPI_Type_create_resized(subtype_B_tmp, 0, sub_n3 * sizeof(double), &subtype_B); // MPI_TYPE_CREATE_RESIZED(oldtype, lower bound, extent, newtype)
    MPI_Type_commit(&subtype_B);
    MPI_Type_free(&subtype_B_tmp);
    if (coords[0] == 0) {
        MPI_Scatter(B.data(), 1, subtype_B, subB.data(), n2 * sub_n3, MPI_DOUBLE, 0, rowComm);
    }
    MPI_Bcast(subB.data(), n2 * sub_n3, MPI_DOUBLE, 0, columnComm);

    multiplySubmatrices(subA, subB, subC, sub_n1, n2, sub_n3);

    MPI_Datatype block_type_temp;
    MPI_Type_vector(sub_n1, sub_n3, n3, MPI_DOUBLE, &block_type_temp);
    MPI_Datatype block_type;
    MPI_Type_create_resized(block_type_temp, 0, sub_n3 * sizeof(double), &block_type);
    MPI_Type_commit(&block_type);
    MPI_Type_free(&block_type_temp);

    std::vector<int> recvcounts(size, 1);
    std::vector<int> displs(size, 0);
    if (rank == 0) {
        for (int r = 0; r < size; r++) {
            int procCoords[2];
            MPI_Cart_coords(cartComm, r, 2, procCoords);
            int i = procCoords[0];
            int j = procCoords[1];
            displs[r] = i * sub_n1 * dims[1] + j;
        }
    }

    MPI_Gatherv(subC.data(), sub_n1 * sub_n3, MPI_DOUBLE, (coords[0] == 0 && coords[1] == 0 ? C.data() : nullptr),
                recvcounts.data(), displs.data(), block_type, 0, cartComm);

    MPI_Type_free(&block_type);

    if (rank == 0) {
        endTime = MPI_Wtime();
        std::cout << "Result: " << (endTime - startTime) << " sec." << std::endl;
		
        /*double maxDiff = getMaxDiffWithCorrectAnswer(A, B, C, n1, n2, n3);     
        std::cout << std::fixed << std::setprecision(7);
        std::cout << "Max difference: " << maxDiff << std::endl;*/
    }
    MPI_Finalize();
    return 0;
}