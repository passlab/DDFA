#include <omp.h>

void axpy_kernel(int N, float *Y, float *X, float a) {
    int i;
    #pragma omp parallel for shared(N, X, Y, a) private(i)
    for (i = 0; i < N; ++i)
        Y[i] += a * X[i];
}