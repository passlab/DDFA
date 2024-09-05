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
