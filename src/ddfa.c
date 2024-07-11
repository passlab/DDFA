#include "ddfa.h"
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef USING_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

#define DATA_MAP_BUFFER_SIZE 256
#define SYMBOL_TABLE_SIZE 128
#define CALL_OBJECT_BUFFER_SIZE 256

__thread int thread_id = 0;
__thread data_map_t data_map_buffer[DATA_MAP_BUFFER_SIZE];
__thread int num_maps;
__thread volatile call_t * top; //The top of call stack of this thread
__thread call_t * root; //The root of call stack of all //which is the call to main function
symbol_t sym_table[SYMBOL_TABLE_SIZE];
__thread int call_depth = 0;

__thread call_t call_buffer[CALL_OBJECT_BUFFER_SIZE];
__thread int num_calls = 0; //number of unique call paths per thread

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

/**
 * The function attach the current call path to the cactus stack. This scenario is for the situation that
 * not every function call are automatically traced.
 *
 * @param root: the root node of the callpath
 * @param runtime_depth: the call path depth from the user code to this point. E.g. If the
 *                       user code call this func, runtime_depth is 1. If user code call another
 *                       func, e.g. map_data, which then call this func, runtime_depth is 2
 */
call_t * attach_callpath(call_t * root, int runtime_depth) {
	void * call_site[MAX_CALLPATH_DEPTH];
	void * func[MAX_CALLPATH_DEPTH];

	//runtime_depth as the index to the backtrace return
	//should be the call_site of the top node of the user callgraph
	int depth = backtrace (call_site, MAX_CALLPATH_DEPTH);
	//assert to make sure the root->call_site is the same as the deepest of backtrace

	//3 because of libc calling convention: _start+32 ==> __libc_start_main_impl+128 ==> __libc_start_call_main+128 ==> main
    //might be different level depending on the ABI/libc
	//int libc_calldepth = 3;

	int i = depth - 1;
	//the call_site of the address of the caller that makes the call
	//This search finds the main function in the callpath
	while (root->call_site != call_site[i]) i--;

	i--; //i now is the index of the callpath that has the first call from main

#ifdef USING_LIBUNWIND
    unw_cursor_t cursor; unw_context_t uc;
    unw_proc_info_t func_info;
    unw_word_t ip;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    int j = 0;
    while (unw_step(&cursor) > 0 && j <= runtime_depth); //ignore runtime calls

    while (unw_step(&cursor) > 0) {
      unw_get_reg(&cursor, UNW_REG_IP, &ip);
      unw_get_proc_info(&cursor, &func_info);
      printf ("call_site = %lx, func = %lx\n", (long) ip, (long) func_info.start_ip);
      call_site[j] = (void *) ip;
      func[j] = (void *) func_info.start_ip;
      j++;
    }
#endif

	call_t * ntop = root;
	for ( ; i>=0; i--) {
		void * ip = call_site[i];
		call_t * child = ntop->child;
		while (child != NULL) {
			if (child->call_site == ip) {
				ntop = child; //The callee is already in the call graph, move on to the next level
				break;
			} else {
				child = child->next; //search the callees to find the matching node
			}
		}
		if (child == NULL) {
			//No child matching the call, this call has not been in the call graph,
			//Thus we need to build the graph from this point (ntop)
			break;
		}
	}
	for (; i > runtime_depth; i--) {
		//build the rest of the nodes in the callpath
		//We do not have func info for the callee
		ntop = add_new_call_node(ntop, NULL, call_site[i]);
	}
	top = ntop;
}

void retrieve_callpath(callpath_key_t * cpk) {
  cpk->thread_id = thread_id;
  cpk->depth = backtrace (cpk->callpath, MAX_CALLPATH_DEPTH);
}

data_map_t * map_data(data_map_t * src, map_type_t mapType, char * symbol, void * addr, size_t size,
		access_kind_t accessKind, mem_type_t memType, int devId) {
	data_map_t * map = &data_map_buffer[num_maps]; num_maps++;
	map->symbol = symbol;
	map->addr = addr;
	map->size = size;
	map->accessKind = accessKind;
	map->devId = devId;
	map->memType = memType;

	map->src = src;
	map->mapType = mapType;
	//callpath_key_t callpath_key;

	attach_callpath(root, 2);
	//attach the call path to the call graph

	//attach the map to the call graph
	//Assume top is the function that makes this map
	map->next = top->data_maps;
	top->data_maps = map; //XXX: Is there data racing in this situation??
}

// remove and return first item, move up all other items
// needs manual deincrement of end
call_t *dequeue(call_t *queue[], int end) {
  if (end > 0) {
    call_t *item = queue[0];
    for (int x = 0; x < end; x++) {
      queue[x] = queue[x + 1];
    }
    // clear last item
    queue[end - 1] = NULL;
    return item;
  }
  return NULL;
}
// add item at first empty slot
// needs manual increment of end
void enqueue(int end, call_t *queue[], call_t *node) { queue[end] = node; }

void __cyg_profile_func_enter(void *this_fn, void *call_site) {
  // we can bootstrap here to create the very first node for call to main
  if (top == NULL) {
    // assert root == NULL, and this_fn == &main
    root = &call_buffer[num_calls];
    num_calls++; // use an object from buffer
    root->count = 1;
    root->func = this_fn;
    root->call_site = call_site;
    root->parent = NULL;
    root->tid = thread_id;
    root->child = NULL;
    root->data_maps = NULL;
    top = root;
    call_depth = 0;
    printf("**ROOT node %p created for calling main: %p, from %p\n", root,
           this_fn, call_site);
  } else {
    call_depth++;
    //top is current parent node of entered funct, location in call stack
    //global var updated at each entry and exit

    // Search entire callstack starting from root
    call_t *parent = root;
    call_t *child;

    //arbitrary size
    call_t *queue[20] = {NULL};
    int end = 0;
    int found = 0; // flag to indicate whether we found the matching node

    enqueue(end, queue, parent);
    end++;

    while ((parent = dequeue(queue, end)) != NULL && !found) {
      end--;
      child = parent->child;

      while (child != NULL) {
        //compare to child, break if match
        //child pointer remains at matching child
        if (child->func == this_fn && child->call_site == call_site && child->tid == thread_id) {

          found = 1;
          break;
        }
        // push the child node to the queue, if still not found
        enqueue(end, queue, child);
        end++;
        child = child->next;
      }
    }
    //No match found, create a new node
    if (child == NULL) {
      child = add_new_call_node(top, this_fn, call_site);
      // for pretty print
      int i;
      for (i = 0; i < call_depth; i++) printf("  ");
      printf("**new call node/path %p created for calling: %p, from %p\n", child,
             this_fn, call_site);
    }
    //count incrments regardless of whether we found a match
    child->count++;
    top = child;
  }
  // for pretty print
  //top is now the matching child node or a new node
  int i;
  for (i = 0; i < call_depth; i++)
    printf("  ");
  printf("==> Call node/path %p entered %d times for calling: %p, from %p\n",
         top, top->count, this_fn, call_site);

} /* __cyg_profile_func_enter */


void __cyg_profile_func_exit(void *this_fn, void *call_site) {
  //for pretty print
  int i;
  for (i=0; i<call_depth; i++) printf("  ");
  printf("<== Call node/path %p exits for calling: %p, from %p, new top: %p\n", top, this_fn, call_site, top->parent);

  call_depth --;
  top = top->parent;
} /* __cyg_profile_func_enter */

extern int main(int argc, char * argv);

void before_main () {
	//root = add_new_call_node(NULL, &main, &before_main);

	root  = &call_buffer[num_calls]; num_calls++; //use an object from buffer
	root->count = 0;
	root->func = &main;
	root->call_site = &before_main;
	root->parent = NULL;
	root->tid = thread_id;
	root->child = NULL;
	root->data_maps = NULL;
	top = root;
	call_depth = 0;
    printf("**ORIGIN node for the program created, get ready for calling main %p, this func is constructor before_main %p**\n", &main, &before_main);
}


void printnode(call_t *node){
  if (node==root){
    printf("\nROOTNODE is %p\n", root);
  } else{
    printf("\nNODEPATH is %p\n", node);
  }
  printf("\tCALLS FUNC %p\n", node->func);
  printf("\t%d times\n", node->count);
  printf("\tFROM CALLSITE: %p\n", node->call_site);
  printf("\tPARENT --> CHILD NODE: %p -> %p\n", node->parent, node->child);
  printf("\tNEXT: %p\n", node->next);
}

void after_main() {
  // // DOT/GraphML implementation
  call_t *parent;
  call_t *child;

  // Size is currently some arbitrary number that is big enough
  call_t *queue[30] = {NULL};
  int end = 0;

  enqueue(end, queue, root);
  end++;

   FILE *fp = fopen("graph.dot", "w");
   fprintf(fp, "digraph callgraph {\n");

  // Add main to dot graph
   fprintf(fp, "_%p;\n", root->func);

  printnode(root);

  while ((parent = dequeue(queue, end)) != NULL) {

    end--;

    child = parent->child;
    while (child != NULL) {
      // create graph node for the child
      // Node names can't start w/nums
      printnode(child);
      // Add child node
      // TODO add if child has not been added before
       fprintf(fp, "_%p;\n", child->func);
      // add connection to parent
       fprintf(fp, "_%p -> _%p [label=\" calls: %d, callsite: %p\"];\n",
       parent->func, child->func, child->count, child->call_site);

      // push the child node to the queue
      enqueue(end, queue, child);
      end++;
      child = child->next;
    }
  }

   fprintf(fp, "}\n");
   fclose(fp);

  // Create the png from dot in terminal
  // dot -T png graph.dot -o graph.png
}
