#include <iostream>
#include <chrono>

class Matrix
{
public:
    static double avg(const double* arr, int trials) {
        double s = 0.0;
        for (int i = 0; i < trials; ++i)
            s += arr[i];
        return s / trials;
    }

    static double* allocateContig(const int N)
    {
        return new double[N*N];
    }

    static void fillContig(const int N, double* contig)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                contig[i*N+j] = rand() % 10 + 1;
            }
        }
    }

    static void deallocateContig(const double* ptr)
    {
        delete[] ptr;
    }

    static double ** allocateRows(const int N)
    {
        auto ** ptr = new double*[N];
        for (int i = 0; i < N; i ++)
        {
            ptr[i] = new double[N];
        }
        return ptr;
    }

    static void fillRows(const int N, double **ptr)
    {
        for (int i = 0; i < N; i ++)
        {
            for (int j = 0; j < N; j ++)
            {
                *(*(ptr + i) + j) = rand() % 10 + 1;
            }
        }
    }

    static void deallocateRows(double** ptr, const int N)
    {
        for (int i = 0; i < N; i ++)
        {
            delete[] ptr[i];
        }
        delete[] ptr;
    }

    static void matmulContig(const double* A, const double* B, double* C, int N)
    {
        for (int i = 0; i < N; i ++)
        {
            for (int j = 0; j < N; j ++)
            {
                C[i * N + j] = 0;
                for (int k = 0; k < N; k ++)
                {
                    C[i * N + j] += A[i * N + k] * B[k * N + j];
                }
            }
        }

    }
    static void matmulRows (double** A, double** B, double** C, int N)
    {
        for (int i = 0; i < N; i ++)
        {
            for (int j = 0; j < N; j ++)
            {
                *(*(C + i) + j) = 0.0;
                for (int k = 0; k < N; k ++)
                {
                    // *(*(C + i) + j ) += (*(*(A+i)+k)) * (*(*(B+k)+j));
                    C[i][j] += A[i][k]*B[k][j];
                }
            }
        }
    }
};

int main()
{
    srand(12345);
    using clk = std::chrono::steady_clock;
    using ms  = std::chrono::duration<double, std::milli>;
    double checksum = 0.0;

    const int N1 = 500;
    const int N2 = 1000;
    constexpr int trials = 5;

    // allocate matrices
    double* contigA1 = Matrix::allocateContig(N1);
    double* contigB1 = Matrix::allocateContig(N1);
    double* contigC1 = Matrix::allocateContig(N1);
    double* contigA2 = Matrix::allocateContig(N2);
    double* contigB2 = Matrix::allocateContig(N2);
    double* contigC2 = Matrix::allocateContig(N2);

    double** rowsA1 = Matrix::allocateRows(N1);
    double** rowsB1 = Matrix::allocateRows(N1);
    double** rowsC1 = Matrix::allocateRows(N1);
    double** rowsA2 = Matrix::allocateRows(N2);
    double** rowsB2 = Matrix::allocateRows(N2);
    double** rowsC2 = Matrix::allocateRows(N2);

    // fill matrices
    Matrix::fillContig(N1, contigA1);
    Matrix::fillContig(N1, contigB1);
    Matrix::fillContig(N2, contigA2);
    Matrix::fillContig(N2, contigB2);

    Matrix::fillRows(N1, rowsA1);
    Matrix::fillRows(N1, rowsB1);
    Matrix::fillRows(N2, rowsA2);
    Matrix::fillRows(N2, rowsB2);

    // untimed warmup
    Matrix::matmulRows(rowsA1, rowsB1, rowsC1, N1);
    Matrix::matmulRows(rowsA2, rowsB2, rowsC2, N2);
    Matrix::matmulContig(contigA1, contigB1, contigC1, N1);
    Matrix::matmulContig(contigA2, contigB2, contigC2, N2);

    // trials
    double rowsResults1[trials];
    double contigResults1[trials];
    double rowsResults2[trials];
    double contigResults2[trials];

    for (int t = 0; t < trials; ++t) {
        // rows N1
        auto t0 = clk::now();
        Matrix::matmulRows(rowsA1, rowsB1, rowsC1, N1);
        auto t1 = clk::now();
        rowsResults1[t] = std::chrono::duration_cast<ms>(t1 - t0).count();
        checksum += rowsC1[t % N1][t % N1];

        // contig N1
        t0 = clk::now();
        Matrix::matmulContig(contigA1, contigB1, contigC1, N1);
        t1 = clk::now();
        contigResults1[t] = std::chrono::duration_cast<ms>(t1 - t0).count();
        checksum += contigC1[(t % N1)*N1 + (t % N1)];

        // rows N2
        t0 = clk::now();
        Matrix::matmulRows(rowsA2, rowsB2, rowsC2, N2);
        t1 = clk::now();
        rowsResults2[t] = std::chrono::duration_cast<ms>(t1 - t0).count();
        checksum += rowsC2[t % N2][t % N2];              // FIX: use N2

        // contig N2
        t0 = clk::now();
        Matrix::matmulContig(contigA2, contigB2, contigC2, N2);
        t1 = clk::now();
        contigResults2[t] = std::chrono::duration_cast<ms>(t1 - t0).count();
        checksum += contigC2[(t % N2)*N2 + (t % N2)];     // FIX: use contigC2 and N2
    }

    std::cout << checksum << std::endl;
    std::cout << "N = " << N1 << "\n";
    std::cout << "  rows   avg: " << Matrix::avg(rowsResults1, trials)   << " ms\n";
    std::cout << "  contig avg: " << Matrix::avg(contigResults1, trials) << " ms\n";

    std::cout << "N = " << N2 << "\n";
    std::cout << "  rows   avg: " << Matrix::avg(rowsResults2, trials)   << " ms\n";
    std::cout << "  contig avg: " << Matrix::avg(contigResults2, trials) << " ms\n";

    // cleanup
    Matrix::deallocateContig(contigA1);
    Matrix::deallocateContig(contigB1);
    Matrix::deallocateContig(contigC1);
    Matrix::deallocateContig(contigA2);
    Matrix::deallocateContig(contigB2);
    Matrix::deallocateContig(contigC2);

    Matrix::deallocateRows(rowsA1, N1);
    Matrix::deallocateRows(rowsB1, N1);
    Matrix::deallocateRows(rowsC1, N1);
    Matrix::deallocateRows(rowsA2, N2);
    Matrix::deallocateRows(rowsB2, N2);
    Matrix::deallocateRows(rowsC2, N2);

    return 0;
}