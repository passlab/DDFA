#include <stdio.h>
#include <stdlib.h>
#include "datamap.h"

int main()
{

    // This pointer will hold the
    // base address of the block created
    int* ptr;
    int n = 100, i;

    int arr1[n]; //dynmaically allocated array on the stack
    data_map_t * arr1Map = map_data(NULL, "arr1", arr1, sizeof(int)*n, 0, 0, 
        ACCESS_KIND_UNKNOWN, MAP_TYPE_INIT, MAP_CLASS_DECL, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);

    // Get the number of elements for the array
    printf("Enter number of elements:");
    scanf("%d",&n);
    printf("Entered number of elements: %d\n", n);

    // Dynamically allocate memory using malloc() on the heap
    ptr = (int*)malloc(n * sizeof(int));
    data_map_t * ptrMap = map_data(NULL, "ptr", ptr, sizeof(int)*n, 0, 0, 
        ACCESS_KIND_UNKNOWN, MAP_TYPE_INIT, MAP_CLASS_malloc, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);

    memcpy(ptr, arr1, sizeof(int)*n);
    link_map(ptrMap, arr1Map);

    // Check if the memory has been successfully
    // allocated by malloc or not
    if (ptr == NULL) {
        printf("Memory not allocated.\n");
        exit(0);
    }
    else {

        // Memory has been successfully allocated
        printf("Memory successfully allocated using malloc.\n");

        // Get the elements of the array
        for (i = 0; i < n; ++i) {
            ptr[i] = i + 1;
        }

        // Print the elements of the array
        printf("The elements of the array are: ");
        for (i = 0; i < n; ++i) {
            printf("%d, ", ptr[i]);
        }
    }

    return 0;
}

int foo(int * dest, int * src, int size) {
    data_map_t * destParaMap = map_data(NULL, "dest", &dest, sizeof(int*), 0, 0, 
     ACCESS_KIND_UNKNOWN, MAP_TYPE_FUNCARGCOPY, TRACE_KIND_PER_CALLPATH, MEM_TYPE_HOSTMEM,  0);
    func_para_map(&foo, destParaMap, 0);
     data_map_t * srcParaMap = map_data(NULL, "src", &src, sizeof(int*), 0, 0, 
     ACCESS_KIND_UNKNOWN, MAP_TYPE_FUNCARGCOPY, TRACE_KIND_PER_CALLPATH, MEM_TYPE_HOSTMEM,  0);
    func_para_map(&foo, srcParaMap, 0);


    memcpy(dest, src, sizeof(int) * size);
    
    data_map_t * destMap = map_data(NULL, "dest", dest, sizeof(int)*size, 0, 0, 
        ACCESS_KIND_UNKNOWN, MAP_TYPE_INIT, MAP_CLASS_PTRVAR, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);
    map_reference(destMap, destParaMap);
    data_map_t * srcMap = map_data(NULL, "src", src, sizeof(int)*size, 0, 0, 
        ACCESS_KIND_UNKNOWN, MAP_TYPE_INIT, MAP_CLASS_PTRVAR, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);
    map_reference(srcMap, srcParaMap);

    link_map(destMap, srcMap);

}