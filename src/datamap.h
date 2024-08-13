#ifndef __DDFA_DATAMAP_H__
#define __DDFA_DATAMAP_H__

#include "basicdef.h"

#define DATA_MAP_BUFFER_SIZE 256
#define SYMBOL_TABLE_SIZE 128

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
} access_kind_t;

typedef enum map_type {
    MAP_TYPE_COPY,
    MAP_TYPE_SHARED,
    MAP_TYPE_VSHARED,
    MAP_TYPE_FUNCARGCOPY, //Function call, copy
    MAP_TYPE_INIT_CONST, //_INIT_* type are for the first time creation of a data element
} map_type_t;

typedef enum mem_type {
    MEM_TYPE_HOSTMEM,
    MEM_TYPE_ACCMEM,
    MEM_TYPE_STORAGE,
    MEM_TYPE_IO,
} mem_type_t;

/**
 * A data map is for tracing the use of data in multiple memory locations, and
 * by different parallel units, e.g.  tasks, threads, offloading tasks, etc.
 *
 * There are several situation that a data_map is created
 * 1. When memory is allocated, e.g. malloc and cudaMalloc, or array declaration
 * 2. Explicit memcpy, cudaMemcpy, mmap and static array/variable declaration
 * 3. OpenMP directives that use map clause, shared or private clause, i.e.
 *     when a new data environment is created for a task/thread/offloading region
 * 4. function call (for pass by value and mapping of symbols)
 */
typedef struct data_map {
    char * symbol;
    void * addr; //The memory address
    size_t size;
    access_kind_t accessKind;
  map_type_t mapType;
  trace_kind_t traceKind;
  int refcount;
    mem_type_t memType;
    int devId; //heterogeneous device id, -1 for CPU/host memory
    struct callpath_key *key;
    struct data_map *src; //If mtype is shared or vshared, src points to the source data_map.

    struct data_map * next; //The link list of maps of a function
    struct data_map * argnext; //The link list of maps of a function
} data_map_t;

typedef char * symbol_t;
extern __thread data_map_t data_map_buffer[];
extern __thread int num_maps;
extern symbol_t sym_table[];

/**
 *
 */
#endif