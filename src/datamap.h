#ifndef __DDFA_DATAMAP_H__
#define __DDFA_DATAMAP_H__

#include "basicdef.h"

#define DATA_MAP_BUFFER_SIZE 256
#define SYMBOL_TABLE_SIZE 128

/**
 * A data map is for tracing the use of data in multiple memory locations, and
 * by different parallel units, e.g.  tasks, threads, offloading tasks, etc.
 *
 * There are several situations (classes) that a data_map is created
 * 1. When memory is allocated, e.g. malloc and cudaMalloc, or array declaration
 * 2. Explicit memcpy, cudaMemcpy, mmap
 * 3. OpenMP directives that use map clause, shared or private clause, i.e.
 *     when a new data environment is created for a task/thread/offloading region
 * 4. function call (for pass by value and mapping of symbols)

 About tracing data map, the design will allow users to 
 1. To specify which map to be traced
 */

typedef enum data_attr {  
    //data claas
    DATA_CLASS_CONST,
    DATA_CLASS_PVAR,
    DATA_CLASS_ARRAY,
    DATA_CLASS_malloc,
    DATA_CLASS_cudaMalloc,
    DATA_CLASS_cudaMallocManaged,
    DATA_CLASS_FILE,
    //access kind
    ACCESS_KIND_READ_ONLY,
    ACCESS_KIND_WRITE_ONLY,
    ACCESS_KIND_READ_WRITE,
    ACCESS_KIND_UNKNOWN,
    //mem type
    MEM_TYPE_HOSTMEM,
    MEM_TYPE_ACCMEM,
    MEM_TYPE_STORAGE,
    MEM_TYPE_IO,
    //data type (?), so far, no need data type (int, float, etc)
} data_attr_t;

typedef enum data_map_attr {
    //map type
    MAP_TYPE_COPY,
    MAP_TYPE_SHARED,
    MAP_TYPE_VSHARED,
    //map class
    MAP_CLASS_FUNCPARA,
    MAP_CLASS_mmap,
    MAP_CLASS_memcpy,
    MAP_CLASS_cudaMemcpy,
    MAP_CLASS_EXPLICIT_MEMCPY, //separator for memcpy-related 
    MAP_CLASS_OMP_MAP_ALLOC,
    MAP_CLASS_OMP_MAP_TO,
    MAP_CLASS_OMP_MAP_FROM,
    MAP_CLASS_OMP_MAP_TOFROM,
    MAP_CLASS_OMP_SHARED,
    MAP_CLASS_OMP_PRIVATE,
    MAP_CLASS_CONST,

} data_map_attr_t;

typedef int data_class_t;
typedef int access_kind_t;
typedef int mem_type_t;
typedef int map_type_t;
typedef int map_class_t;

//constant from 0 - .. for indexing the map attributes 
typedef enum data_attr_index {
    DATAATTR_data_class,
    DATAATTR_access_kind,
    DATAATTR_mem_type,
    DATAATTR_num_attrs, //total number of attributes
};

//constant from 0 - .. for indexing the map attributes 
typedef enum map_attr_index {
    MAPATTR_map_type,
    MAPATTR_map_class,
    MAPATTR_num_attrs, //total number of attributes
};

typedef struct data_info {
    void * addr; //The memory address
    int devId; //heterogeneous device id, -1 for CPU/host memory
    struct callpath_key *location; //Uniquely identify the location of this map
    int threadId;
        // type/class/kind/function/trace_kind/thread;

    size_t size;
    char * symbol;

    trace_kind_t traceKind;
    int count;

    struct data_attrs {
        data_attr_t attr;
        struct data_info * next;
        struct data_info * prev;
    } data_attrs [DATAATTR_num_attrs];

    struct data_info * next; //pointer for the linked-list of all data_info of a function call

    int index; //reference for example which argu/parameters this is for a function call
    
    struct data_info * next2; //pointer for the temp linked-list of the data_info that will be used next
    void * reference; //temp for the last time this info is used for

} data_info_t;

typedef struct data_map {
    //map information/property. They are used for index in the map_link
    struct data_info *src; //If mtype is shared or vshared, src points to the source data_map.
    struct data_info *dest;
    //In the situation where a pointer is a data map, the mem segment the pointer points to is another map
    //we can use this filed for the mem-segment map to point to the pointer map

    struct callpath_key *location; //Uniquely identify the location of this map
    int threadId; // type/class/kind/function/trace_kind/thread;

    trace_kind_t traceKind;

    //book-keeping updated at runtime
    int count;

    //Each attribute also has double-link list links that are used to index maps of the same attributes. 
    //In this way, we have a cross-link such that maps can be efficiently found
    struct map_attrs {
        data_map_attr_t attr;
        struct data_map * next;
        struct data_map * prev;
    } map_attrs [MAPATTR_num_attrs];

    struct data_map * next; //The link list of all maps of a function
    struct data_map * next2; //the link list for the mapped argument of a function call
} data_map_t;

typedef char * symbol_t;
extern __thread data_map_t data_map_buffer[];
extern __thread int num_maps;
extern symbol_t sym_table[];

extern data_info_t *init_data_info(char *symbol, void *addr, size_t size, int devId, data_class_t dataClass,
     access_kind_t accessKind, mem_type_t memType, trace_kind_t traceKind, int count);
extern void add_func_argu(void * func, data_info_t * info, int index) ;
extern data_map_t * map_funccall_argu(void * func, data_info_t * para, int index);
extern void end_map_funccall_argu ();

/**
 *
 */
#endif
