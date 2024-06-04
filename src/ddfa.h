#ifndef __DDFA_DDFA_H__
#define __DDFA_DDFA_H__

#include <libunwind.h>

typedef enum data_type {
	DATA_TYPE_char,
	DATA_TYPE_short, //signed
	DATA_TYPE_int,   //signed
	DATA_TYPE_long,   //signed
	DATA_TYPE_float,
	DATA_TYPE_double,
	DATA_TYPE_uint32,
	DATA_TYPE_uint64,
}data_type_t;

typedef enum access_kind {
	ACCESS_KIND_READ_ONLY,
	ACCESS_KIND_WRITE_ONLY,
	ACCESS_KIND_READ_WRITE,
} access_kind_t;

typedef enum map_type {
	MAP_TYPE_COPY,
	MAP_TYPE_SHARED,
	MAP_TYPE_VSHARED,
	MAP_TYPE_FUNCOPY, //Function call, copy
} map_type_t;

#define MAX_CALLPATH_DEPTH 32
//This is used a key to uniquely identify a data or a data map
typedef struct callpath_key {
	int thread_id;
	void * callpath[MAX_CALLPATH_DEPTH];
	int depth;
} callpath_key_t;

typedef char * symbol_t;

typedef struct call {
	void * func;
	void * call_site;
	void * parent;
	unsigned int count; //The call count in this call path
	int tid; //The thread id
	volatile struct call * child; //callee
	struct call * next; //The link list for the children (callees)
} call_t;

/**
 * A data is created when there is malloc or cudaMalloc, or array declaration
 * (including struct of array or array of struct).
 * Or new data map is created, in which the new data is created for the map
 *
 * We use data instead of memory (memory segment) to be more related to application
 */
typedef struct data {
	char * symbol;
	void * addr; //The memory address
	size_t size;
	access_kind_t akind;
	int devId; //heterogeneous device id, -1 for CPU/host memory
	callpath_key_t key;
} data_t;

/**
 * A data map is for tracing the use of data in multiple memory locations, and
 * by different parallel units, e.g.  tasks, threads, offloading tasks, etc.
 *
 * There are several situation that a data_map is created
 * 1. Explicit memcpy, cudaMemcpy, mmap and static array/variable declaration
 * 2. OpenMP directives that use map clause, shared or private clause, i.e.
 *     when a new data environment is created for a task/thread/offloading region
 * 3. function call (for pass by value and mapping of symbols)
 */
typedef struct data_map {
	data_t data;
	data_t *src; //If mtype is shared or vshared, dst is pointer to data.
	map_type_t mtype;
	callpath_key_t key;
} data_map_t;

/**
 *
 */
data_t * init_data(char *symbol, void *addr, size_t size, access_kind_t akind, int devId);
data_map_t * init_map_data(data_t * src, map_type_t mtype, char * symbol, void * addr, size_t size, access_kind_t akind, int devID);
data_map_t * map_data(data_t * src, data_t * dest, map_type_t mtype);

extern __thread thread_id;
extern __thread data_buffer[];
extern __thread num_data;
extern __thread data_map_buffer[];
extern __thread num_maps;
extern symbol_t sym_table[];

/**
 * @input symbol: optional symbol for the the mem segment
 * @input addr: address
 * @input size: size
 * @input type: data type
 * @input akind: access kind
 */
void trace_mem(char * symbol, void * addr, size_t size, data_type_t type, access_kind_t * akind, int dev) {

}

#define DDF_TRACE_START(numsyms, ...) ddf_trace_start(__func__, "", numsyms, __VA_ARGS__)
#define DDF_TRACE_START_FUNC(funcName, numsyms, ...) ddf_trace_start(__func__, funcName, numsyms, __VA_ARGS__)
#define SYM_TRACE_SIMPLE(SYM, type, akind, dev) #SYM, &SYM, sizeof(SYM), type, akind, dev

/**
 * @input callerName: the func name that has this trace call
 * @input funcName: If it is tracing a function call, this is the func symbol
 * @input numsyms: number of symbols to be traced
 */
void ddf_trace_start(char * callerName, char * funcName, int numsyms, ...) {
}

#endif

