## Performance Improvements

The optimization reduced the execution time from approximately **70 milliseconds** to **17 milliseconds**. The improvements come from several key changes:

### 1. Removing Function Call Overhead
The original implementation used `getElement()` inside the nested loops. Calling a function for every element access introduces unnecessary overhead and makes it harder for the compiler to optimize the loop.

By retrieving a pointer to the row and accessing elements directly, all function calls are eliminated.

### 2. Improved Cache Locality
Using:

const int* rowPtr = matrix[i].data();

Allows the loop to iterate through memory linearly. This access pattern improves cache locality because sequential data is loaded into cache together. As a result, subsequent accesses within the same row are significantly faster.


### 3. Cheaper Pointer Indexing

Indexing through a raw pointer (e.g., rowPtr[j]) is cheaper than indexing through std::vector, which involves additional bounds checking and address calculations.

Pointer arithmetic provides a simpler, more predictable access pattern, and makes it easier for the compiler to optimize or auto-vectorize the inner loop.
