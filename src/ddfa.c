#include "ddfa.h"

#define DATA_BUFFER_SIZE 256
#define DATA_MAP_BUFFER_SIZE 256
#define SYMBOL_TABLE_SIZE 128
#define CALL_OBJECT_BUFFER_SIZE 256

__thread thread_id;
__thread data_buffer[DATA_BUFFER_SIZE];
__thread int num_data;
__thread data_map_buffer[DATA_MAP_BUFFER_SIZE];
__thread int num_maps;
__thread call_t * top; //The top of call stack of this thread
__thread call_t * root; //The root of call stack of all //which is the call to main function
symbol_t sym_table[SYMBOL_TABLE_SIZE];

__thread call_t call_buffer[CALL_OBJECT_BUFFER_SIZE];
__thread num_calls = 0; //number of unique call paths per thread

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void __cyg_profile_func_enter(void *this_fn, void *call_site)
                              __attribute__((no_instrument_function));


call_t * add_new_call_node(call_t * parent, void * func, void * call_site) {
  call_t * temp = &call_buffer[num_calls]; num_calls++; //use an object from buffer
  temp->count = 0;
  temp->func = func;
  temp->call_site = call_site;
  temp->parent = parent;
  temp->tid = thread_id;
  temp->child = NULL;

  //Attach to the cactus stack call path. The order of listing callee can be in either
  //the order of function calls or in the order of their appearance in the code. For the first
  //way of ordering, we just append or prepend the callee to the end of the next pointer.
  //For the second way of ordering, we need to use the call_site address to compare.
  temp->next = parent->child; //prepend
  parent->child = temp; //XXX TODO: data racing if two threads are updating the cactus stack
  return temp;
}

void __cyg_profile_func_enter(void *this_fn, void *call_site) {
  printf("ENTER: %p, from %p\n", this_fn, call_site);
  //Search the callee of the current parent to see whether this is called before
  call_t * temp = top->child;
  while (temp != NULL) {
	  if (temp->func == this_fn && temp->call_site == call_site && temp->tid == thread_id) {
		  break;
	  }
  }
  if (temp == NULL) {
	  temp = add_new_call_node(top, this_fn, call_site);
  }
  temp->count ++;
  top = temp;

} /* __cyg_profile_func_enter */

void __cyg_profile_func_exit(void *this_fn, void *call_site)
                             __attribute__((no_instrument_function));
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
  printf("EXIT:  %p, from %p\n", this_fn, call_site);
  top = top->parent;
} /* __cyg_profile_func_enter */

/**
 * The function attach the current call path to the cactus stack
 */
call_t * attach_callpath(call_t * root) {
	void * callpath[MAX_CALLPATH_DEPTH];
	int depth = backtrace (callpath, MAX_CALLPATH_DEPTH);
	int i;
	//assert to make sure the root->call_site is the same as the deepest of backtrace
	if (root->call_site != callpath[depth-1]) { //might be different level depending on the ABI/libc
	}
	call_t * ntop = root;
	for (i = depth - 2; i>=0; i--) {
		void * ip = callpath[i];
		call_t * child = ntop->child;
		while (child != NULL && child != ip) {
			child = child->next;
		}
		if (child == ip) {
			ntop = child; //find the associated call node, continue the next one
		} else { //we need to break since from now on, we need to build all the call nodes in
			     //the rest of the callpath
			break;
		}
	}
	if (i>=0) { //build the rest of the nodes in the callpath
		for (; i>=0; i++) {
			ntop = add_new_call_node(ntop, NULL, callpath[i]);
		}
	}
	top = ntop;
}

void retrieve_callpath(callpath_key_t * cpk) {
  cpk->thread_id = thread_id;
  cpk->depth = backtrace (cpk->callpath, MAX_CALLPATH_DEPTH);
}

data_t * init_data(char *symbol, void *addr, size_t size, access_kind_t akind, int devId) {
	callpath_key_t callpath_key;
	data_t * data = data_buffer[num_data]; num_data++;
	data->symbol = symbol;
	data->addr = addr;
	data->size = size;
	data->akind = akind;
	data->devId = devId;
}
data_map_t * init_map_data(data_t * src, map_type_t mtype, char * symbol, void * addr, size_t size, access_kind_t akind, int devID);
data_map_t * map_data(data_t * src, data_t * dest, map_type_t mtype);
