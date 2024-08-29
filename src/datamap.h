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

typedef enum data_type {
    DATA_TYPE_char,
    DATA_TYPE_short, //signed
    DATA_TYPE_int,   //signed
    DATA_TYPE_long,   //signed
    DATA_TYPE_float,
    DATA_TYPE_double,
    DATA_TYPE_uint32,
    DATA_TYPE_uint64,
} data_type_t;

typedef enum access_kind { 
    ACCESS_KIND_READ_ONLY,
    ACCESS_KIND_WRITE_ONLY,
    ACCESS_KIND_READ_WRITE,
    ACCESS_KIND_UNKNOWN,
} access_kind_t;

typedef enum map_type { //sharing kind
    MAP_TYPE_COPY,
    MAP_TYPE_SHARED,
    MAP_TYPE_VSHARED,
    MAP_TYPE_FUNCARGCOPY, //Function call, copy
    MAP_TYPE_INIT_CONST, //_INIT_* type are for the first time creation of a data element
} map_type_t;

typedef enum map_class {
    MAP_CLASS_DECL,  //variable/array declaration
    MAP_CLASS_malloc,
    MAP_CLASS_mmap,
    MAP_CLASS_cudaMalloc,
    MAP_CLASS_cudaMallocManaged,
    MAP_CLASS_MEMALLOC, //separator thus those declared before are mem allocation related
    MAP_CLASS_memcpy,
    MAP_CLASS_cudaMemcpy,
    MAP_CLASS_EXPLICIT_MEMCPY, //separator for memcpy-related 
    MAP_CLASS_OMP_MAP_ALLOC,
    MAP_CLASS_OMP_MAP_TO,
    MAP_CLASS_OMP_MAP_FROM,
    MAP_CLASS_OMP_MAP_TOFROM,
    MAP_CLASS_OMP_SHARED,
    MAP_CLASS_OMP_PRIVATE,
} map_class_t;

typedef enum mem_type {
    MEM_TYPE_HOSTMEM,
    MEM_TYPE_ACCMEM,
    MEM_TYPE_STORAGE,
    MEM_TYPE_IO,
} mem_type_t;

//constant from 0 - .. for indexing the map_links 
typedef enum map_link_index {
    MAPLINKINDEX_trace_kind,
    MAPLINKINDEX_acces_kind,
    MAPLINKINDEX_map_type,
    MAPLINKINDEX_map_class,
    MAPLINKINDEX_mem_type,
    MAPLINKINDEX_addr,
    MAPLINKINDEX_devId,
    MAPLINKINDEX_src,
    MAPLINKINDEX_location,
    MAPLINKINDEX_NUMLINKS, //Must be the last one
};

typedef struct data_map {
    //map information/property
    struct data_map *src; //If mtype is shared or vshared, src points to the source data_map.
    char * symbol;
    void * addr; //The memory address
    size_t size;
    int devId; //heterogeneous device id, -1 for CPU/host memory
    struct data_map *src; //If mtype is shared or vshared, src points to the source data_map.
    struct callpath_key *location; //Uniquely identify the location of this map
    int index; //local index if there is multiple maps at the same location, e.g. function call argument

    //map attribute
    access_kind_t accessKind;
    map_type_t mapType;
    trace_kind_t traceKind;
    mem_type_t memType;

    //book-keeping updated at runtime
    int count;

    //map links are used to link maps of the same type/class/kind/function/trace_kind;
    //In this way, we have a cross-link such that maps can be effiencelty find and organized
    struct map_link {
        struct data_map * next;
        struct data_map * prev;
    } map_links [MAPLINKINDEX_NUMLINKS];

    struct data_map * next; //The link list of maps of a function
    struct data_map * argnext; //The link list of maps of a function
} data_map_t;

typedef char * symbol_t;
extern __thread data_map_t data_map_buffer[];
extern __thread int num_maps;
extern symbol_t sym_table[];

extern data_map_t *map_data(data_map_t *src, char *symbol, void *addr, size_t size, int devId,  
     access_kind_t accessKind, map_type_t mapType, trace_kind_t traceKind, mem_type_t memType,  int count);
extern void func_arg_map (void * fp, data_map_t * argMap, int index);

/**
 *
 */
#endif