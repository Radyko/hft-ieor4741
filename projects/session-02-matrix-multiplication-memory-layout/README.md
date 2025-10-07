### Matrix Multiplication: Contiguous vs Row-Pointer Layouts

#### Overview
This project benchmarks two different memory layouts for dense matrix multiplication in C++:

Contiguous layout (double*) — a single N×N block of memory.

Row-pointer layout (double**) — an array of N pointers, each pointing to a row of N doubles.

Both use the naïve triple-nested loop algorithm for multiplying two matrices A and B into C.

The program allocates and fills matrices with random integers (1–10), performs one untimed warm-up, then runs five timed trials for N=500 and N=1000, computing and printing the average runtime for each layout. A checksum is accumulated to prevent the compiler from optimizing away the computations.

#### Build Instructions
Linux / macOS (GCC or Clang):

<pre>
bash g++ -std=c++17 -O3 -march=native -Wall -Wextra -pedantic main.cpp -o matmul
./matmul
</pre>


#### Example Output:
462123 // this is checksum

N = 500

rows   avg: 199.212 ms

contig avg: 174.794 ms

N = 1000

rows   avg: 1927.84 ms

contig avg: 1570.59 ms

#### Conclusion: 
The performance of matrix multiplication using contiguous memory is noticeably faster than when using row-by-row 
(pointer-based) memory allocation. This is primarily because contiguous arrays provide better cache locality and avoid
the extra pointer dereferencing required for accessing rows.