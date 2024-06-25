#ifndef __DDFA_DDFA_H__
#define __DDFA_DDFA_H__

//#include <libunwind.h>
#include <stddef.h>

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

#define MAX_CALLPATH_DEPTH 32
//This is used a key to uniquely identify a data or a data map
typedef struct callpath_key {
	int thread_id;
	void * callpath[MAX_CALLPATH_DEPTH];
	int depth;
} callpath_key_t;

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
	mem_type_t memType;
	int devId; //heterogeneous device id, -1 for CPU/host memory
	callpath_key_t key;
	map_type_t mapType;
	struct data_map *src; //If mtype is shared or vshared, src points to the source data_map.

	struct data_map * next; //The link list of maps of a function
} data_map_t;

typedef char * symbol_t;

typedef struct call {
	void * func;
	void * call_site; //call_site is the address of the caller that makes call to this func
	void * parent;
	unsigned int count; //The call count in this call path
	int tid; //The thread id
	volatile struct call * child; //callee
	volatile struct call * next; //The link list for the children (callees)

	volatile data_map_t *data_maps; //The link list of data maps that are traced in this call
} call_t;

/**
 *
 */
data_map_t * map_data(data_map_t * src, map_type_t mapType, char * symbol, void * addr, size_t size, access_kind_t accessKind, mem_type_t memType, int devId);
call_t * attach_callpath(call_t * root, int runtime_depth) ;

extern __thread int thread_id;
extern __thread int call_depth;
extern __thread data_map_t data_map_buffer[];
extern __thread int num_maps;
extern symbol_t sym_table[];
extern __thread call_t * root;

/**
 * @input symbol: optional symbol for the the mem segment
 * @input addr: address
 * @input size: size
 * @input type: data type
 * @input akind: access kind
 */
void trace_mem(char * symbol, void * addr, size_t size, data_type_t type, access_kind_t * akind, int dev);

#define DDF_TRACE_START(numsyms, ...) ddf_trace_start(__func__, "", numsyms, __VA_ARGS__)
#define DDF_TRACE_START_FUNC(funcName, numsyms, ...) ddf_trace_start(__func__, funcName, numsyms, __VA_ARGS__)
#define SYM_TRACE_SIMPLE(SYM, type, akind, dev) #SYM, &SYM, sizeof(SYM), type, akind, dev

/**
 * @input callerName: the func name that has this trace call
 * @input funcName: If it is tracing a function call, this is the func symbol
 * @input numsyms: number of symbols to be traced
 */
void ddf_trace_start(char * callerName, char * funcName, int numsyms, ...);

void __cyg_profile_func_enter(void *this_fn, void *call_site)
                              __attribute__((no_instrument_function));


void __cyg_profile_func_exit(void *this_fn, void *call_site)
                             __attribute__((no_instrument_function));

//Not using the constructor now
//void before_main () __attribute__((constructor));
void before_main () ;
void after_main () __attribute__((destructor));

#endif

