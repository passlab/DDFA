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
    struct call * parent;
    unsigned int count; //The call count in this call path
    int tid; //The thread id
    volatile struct call * child; //callee
    volatile struct call * next; //The link list for the children (callees)

    volatile struct data_info *data_infos; //The head of the link list of data_info that are created in this call
    volatile struct data_map  *data_maps; //data maps created within this function
    volatile struct data_info * arg_infos; //data maps for arguments
    volatile struct data_map * arg_maps; //args maps

    struct data_info * temp_info; //The head of the linked-list that is using next2 pointer and index of the data_info_t, e.g. the data_info for the arguments of a callee
                                  //This is temporary since when the callee finishes mapping the argument data info with its parameter info, this temp_info will be reset to NULL
                                  //and to be used for the next function call. 

    struct data_map * next_callarg_maps; //The cache to store the callarg_meta for the next callee. The callee node should store this pointer as soon as it can since it will be overwritten for the next call
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

extern void init_before_main() ;
extern void dump_callgraph();

#endif