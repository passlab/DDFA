#include <omp.h>
#include "datamap.h"

void axpy_kernel(int N, float *Y, float *X, float a) {
    int i;

    data_map_t * NMap = map_data(NULL, "N", &N, sizeof(int), 0, 0, 
        ACCESS_KIND_UNKNOWN, MAP_CLASS_VAR, MAP_CLASS_OMP_SHARED, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);
    #pragma omp parallel for shared(N, X, Y, a) private(i)
    for (i = 0; i < N; ++i)
        Y[i] += a * X[i];
}