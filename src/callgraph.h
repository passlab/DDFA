#ifndef __DDFA_CALLPATH_H__
#define __DDFA_CALLPATH_H__

#include "basicdef.h"

#define MAX_CALLPATH_DEPTH 32
//This is used a key to uniquely identify a data or a data map
typedef struct callpath_key {
    int thread_id;
    void * callpath[MAX_CALLPATH_DEPTH];
    int depth;
} callpath_key_t;

typedef struct call {
    void * func;
    void * call_site; //call_site is the address of the caller that makes call to this func
    void * parent;
    unsigned int count; //The call count in this call path
    int tid; //The thread id
    volatile struct call * child; //callee
    volatile struct call * next; //The link list for the children (callees)

    volatile struct data_map *data_maps; //The link list of data maps that are traced in this call

    struct data_map * callarg_meta; //The call argument meta of the caller of this call

    struct data_map * next_callarg_meta; //The cache to store the callarg_meta for the next callee. The callee node should store this pointer as soon as it can since it will be overwritten for the next call
} call_t;


call_t * attach_callpath(call_t * root, int runtime_depth) ;

//extern __thread int thread_id;
extern __thread int call_depth;
extern call_t * root;
extern __thread volatile call_t *top; // The top of call stack of this thread

void __cyg_profile_func_enter(void *this_fn, void *call_site)
                              __attribute__((no_instrument_function));


void __cyg_profile_func_exit(void *this_fn, void *call_site)
                             __attribute__((no_instrument_function));

void init_before_main() ;
void dump_callgraph();

#endif