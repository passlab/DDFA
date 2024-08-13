#include "callgraph.h"

#include <execinfo.h>
#include <omp.h>
//extern int omp_get_thread_num();
#include <stdio.h>
#include <stdlib.h>

#ifdef USING_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

#define CALL_OBJECT_BUFFER_SIZE 2048
//__thread int thread_id = 0;
__thread volatile call_t *top; // The top of call stack of this thread
 call_t *root; // The root of call stack of all //which is the call to main function
__thread int call_depth = 0;

__thread call_t call_buffer[CALL_OBJECT_BUFFER_SIZE];
__thread int num_calls = 0; // number of unique call paths per thread

static call_t *add_new_call_node(call_t *parent, void *func, void *call_site) {
  call_t *temp = &call_buffer[num_calls];
  num_calls++; // use an object from buffer
  temp->count = 0;
  temp->func = func;
  temp->call_site = call_site;
  temp->parent = parent;
  temp->tid = omp_get_thread_num();
  temp->child = NULL;

  // Attach to the cactus stack call path. The order of listing callee can be in
  // either the order of function calls or in the order of their appearance in
  // the code. For the first way of ordering, we just append or prepend the
  // callee to the end of the next pointer. For the second way of ordering, we
  // need to use the call_site address to compare.
  temp->next = parent->child; // prepend
  parent->child = temp; // XXX TODO: data racing if two threads are updating the
                        // cactus stack
  return temp;
}


void __cyg_profile_func_enter(void *this_fn, void *call_site) {
  // we can bootstrap here to create the very first node for call to main
  if (root == NULL) {

    // assert root == NULL, and this_fn == &main
    root = &call_buffer[num_calls];
    num_calls++; // use an object from buffer
    root->count = 1;
    root->func = this_fn;
    root->call_site = call_site;
    root->parent = NULL;
    root->tid = omp_get_thread_num();
    root->child = NULL;
    root->data_maps = NULL;
    top = root;
    call_depth = 0;

    //Add root nodes to queue
    // enqueue(end, queue, root);
    // end++;

    printf("**ROOT node %p created for calling : %p, from %p with tid %d\n\n", root,
    this_fn, call_site, root->tid);
  } else {
    if (top == NULL) 
    top = root;

    call_depth++;
    // Search the callee of the current parent to see whether this is called
    // before
    call_t *temp = top->child;
    while (temp != NULL) {
      //Nodes are distinguished by function pointer, call site address, and thread id
      if (temp->func == this_fn && temp->call_site == call_site &&
          temp->tid == omp_get_thread_num()) {
        break;
      }
      temp = temp->next;
    }
    if (temp == NULL) {
      temp = add_new_call_node(top, this_fn, call_site);
      // for pretty print
      int i;
      for (i=0; i<call_depth; i++) printf("  ");
       printf("**new call node/path %p created for calling: %p, from %p with tid: %d\n", temp, this_fn, call_site, omp_get_thread_num());
    }
    temp->count++;
    top = temp;
  }
  // for pretty print
  // int i;
  // for (i=0; i<call_depth; i++) printf("  ");
  //  printf("==> Call node/path %p entered %d times for calling: %p, from %p\n",
  //  top, top->count, this_fn, call_site);

} /* __cyg_profile_func_enter */

void __cyg_profile_func_exit(void *this_fn, void *call_site) {
  // for pretty print

  // int i;
  // for (i=0; i<call_depth; i++) printf("  ");
  // printf("<== Call node/path %p exits for calling: %p, from %p, new top:  %p\n", top, this_fn, call_site, top->parent);

  call_depth--;
  top = top->parent;
} /* __cyg_profile_func_enter */


/**
 * The function attach the current call path to the cactus stack. This scenario
 * is for the situation that not every function call are automatically traced.
 *
 * @param root: the root node of the callpath
 * @param runtime_depth: the call path depth from the user code to this point.
 * E.g. If the user code call this func, runtime_depth is 1. If user code call
 * another func, e.g. map_data, which then call this func, runtime_depth is 2
 */
call_t *attach_callpath(call_t *root, int runtime_depth) {
  void *call_site[MAX_CALLPATH_DEPTH];
  void *func[MAX_CALLPATH_DEPTH];

  // runtime_depth as the index to the backtrace return
  // should be the call_site of the top node of the user callgraph
  int depth = backtrace(call_site, MAX_CALLPATH_DEPTH);
  // assert to make sure the root->call_site is the same as the deepest of
  // backtrace

  // 3 because of libc calling convention: _start+32 ==>
  // __libc_start_main_impl+128 ==> __libc_start_call_main+128 ==> main
  // might be different level depending on the ABI/libc
  // int libc_calldepth = 3;

  int i = depth - 1;
  // the call_site of the address of the caller that makes the call
  // This search finds the main function in the callpath
  while (root->call_site != call_site[i])
    i--;

  i--; // i now is the index of the callpath that has the first call from main

#ifdef USING_LIBUNWIND
    unw_cursor_t cursor; unw_context_t uc;
    unw_proc_info_t func_info;
    unw_word_t ip;

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    int j = 0;
    while (unw_step(&cursor) > 0 && j <= runtime_depth) j++; //ignore runtime calls

    while (unw_step(&cursor) > 0) {
      unw_get_reg(&cursor, UNW_REG_IP, &ip);
      unw_get_proc_info(&cursor, &func_info);
      printf ("call_site = %lx, func = %lx\n", (long) ip, (long) func_info.start_ip);
      if (func_info.start_ip == root.func) //search till the main func
      call_site[j] = (void *) ip;
      func[j] = (void *) func_info.start_ip;
      j++;
    }
#endif

  call_t *ntop = root;
  for (; i >= 0; i--) {
    void *ip = call_site[i];
    call_t *child = ntop->child;
    while (child != NULL) {
      if (child->call_site == ip) {
        ntop = child; // The callee is already in the call graph, move on to the
                      // next level
        break;
      } else {
        child = child->next; // search the callees to find the matching node
      }
    }
    if (child == NULL) {
      // No child matching the call, this call has not been in the call graph,
      // Thus we need to build the graph from this point (ntop)
      break;
    }
  }
  for (; i > runtime_depth; i--) {
    // build the rest of the nodes in the callpath
    // We do not have func info for the callee
    ntop = add_new_call_node(ntop, NULL, call_site[i]);
  }
  top = ntop;
}

void retrieve_callpath(callpath_key_t *cpk) {
  cpk->thread_id = omp_get_thread_num();
  cpk->depth = backtrace(cpk->callpath, MAX_CALLPATH_DEPTH);
}

extern int main(int argc, char * argv[]);

void init_before_main() {
  // root = add_new_call_node(NULL, &main, &before_main);

  root = &call_buffer[num_calls];
  num_calls++; // use an object from buffer
  root->count = 0;
  root->func = &main;
  root->call_site = &before_main;
  root->parent = NULL;
  root->tid = omp_get_thread_num();
  root->child = NULL;
  root->data_maps = NULL;
  top = root;
  call_depth = 0;
  printf("**ORIGIN node for the program created, get ready for calling main "
         "%p, this func is constructor before_main %p**\n",
         &main, &before_main);
}

//For dumping the callgraph to GRAPHML file
call_t *queue[50] = {NULL};
int end = 0;

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


void printnode(call_t *node) {
  if (node == root) {
    printf("\nROOTNODE is %p\n", root);
  } else {
    printf("\nNODEPATH is %p\n", node);
  }
  printf("\tCALLS FUNC %p\n", node->func);
  printf("\t%d times\n", node->count);
  printf("\tFROM CALLSITE: %p\n", node->call_site);
  printf("\tPARENT --> CHILD NODE: %p -> %p\n", node->parent, node->child);
  printf("\tNEXT: %p\n", node->next);
}

void dump_callgraph() {
//colors array to differentiate thread ids
  char *colors[6] = {"#FF0000", "#FF7F50", "#FFFF00", "#00FF00", "#0000FF", "#00FFFF"};

  // DOT/GraphML implementation
  call_t *parent;
  call_t *child;
  printf("\n\n");
  // Size is currently some arbitrary number that is big enough
  enqueue(end, queue, root);
  end++;
  //Start with filled queue of roots

  FILE *graphml_file = fopen("graph.graphml", "w");

  // XML Header
  fprintf(graphml_file,
          "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
  fprintf(graphml_file,
          "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" "
          "xmlns:java=\"http://www.yworks.com/xml/yfiles-common/1.0/java\" "
          "xmlns:sys=\"http://www.yworks.com/xml/yfiles-common/markup/"
          "primitives/2.0\" "
          "xmlns:x=\"http://www.yworks.com/xml/yfiles-common/markup/2.0\" "
          "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
          "xmlns:y=\"http://www.yworks.com/xml/graphml\" "
          "xmlns:yed=\"http://www.yworks.com/xml/yed/3\" "
          "xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
          "http://www.yworks.com/xml/schema/graphml/1.1/ygraphml.xsd\">\n");

  fprintf(graphml_file,
          "<key id=\"d0\" for=\"node\" yfiles.type=\"nodegraphics\"/>\n<key " "id=\"d1\" for=\"edge\" yfiles.type=\"edgegraphics\"/>\n");

  fprintf(graphml_file, "\t<graph id=\"G\" edgedefault=\"directed\">\n");

  while ((parent = dequeue(queue, end)) != NULL) {
    // Node ID
    fprintf(graphml_file, "\t\t<node id=\"%p-%d-%p\">\n", parent->func, parent->tid, parent->call_site);
    fprintf(graphml_file, "\t\t\t<data key=\"d0\">\n");
    fprintf(graphml_file, "\t\t\t\t<y:ShapeNode>\n");
    // Node Size
    fprintf(graphml_file, "\t\t\t\t\t<y:Geometry x=\"170.5\" y=\"-15.0\" " "width=\"400.0\" height=\"30.0\"/>\n");
    fprintf(graphml_file, "\t\t\t\t\t<y:Fill color=\"%s\" transparent=\"false\"/>\n", colors[parent->tid]);
    fprintf(graphml_file, "\t\t\t\t\t<y:BorderStyle type=\"line\" width=\"1.0\" " "color=\"#000000\"/>\n");
    //Node Label
    fprintf(graphml_file, "\t\t\t\t\t<y:NodeLabel>%p-%d-%p</y:NodeLabel>\n", parent->func, parent->tid, parent->call_site);
    fprintf(graphml_file, "\t\t\t\t\t<y:Shape type=\"rectangle\"/>\n");
    //Closing tags
    fprintf(graphml_file, "\t\t\t\t</y:ShapeNode>\n");
    fprintf(graphml_file, "\t\t\t</data>\n");
    fprintf(graphml_file, "\t\t</node>\n");

    end--;

    child = parent->child;
    while (child != NULL) {
      // Add child node

      // Node ID
      fprintf(graphml_file, "\t\t<node id=\"%p-%d-%p\">\n", child->func, child->tid, child->call_site);
      fprintf(graphml_file, "\t\t\t<data key=\"d0\">\n");
      fprintf(graphml_file, "\t\t\t\t<y:ShapeNode>\n");
      // Node size
      fprintf(graphml_file, "\t\t\t\t\t<y:Geometry x=\"170.5\" y=\"-15.0\" " "width=\"300.0\" height=\"30.0\"/>\n");
      fprintf(graphml_file, "\t\t\t\t\t<y:Fill color=\"%s\" transparent=\"false\"/>\n", colors[child->tid]);
      fprintf(graphml_file, "\t\t\t\t\t<y:BorderStyle type=\"line\" " "width=\"1.0\" color=\"#000000\"/>\n");
      // Label with function pointer
      fprintf(graphml_file, "\t\t\t\t\t<y:NodeLabel>%p-%d-%p</y:NodeLabel>\n", child->func, child->tid, child->call_site);
      fprintf(graphml_file, "\t\t\t\t\t<y:Shape type=\"rectangle\"/>\n");
      //Closing tags
      fprintf(graphml_file, "\t\t\t\t</y:ShapeNode>\n");
      fprintf(graphml_file, "\t\t\t</data>\n");
      fprintf(graphml_file, "\t\t</node>\n");

      // Add connecting edge to parent
      fprintf(graphml_file, "\t\t<edge id=\"%p-%d\" source=\"%p-%d-%p\" target=\"%p-%d-%p\" " "color=\"rgb(0,0,0)\" >\n",
      child, child->tid,
      parent->func, parent->tid, parent->call_site,
      child->func, child->tid, child->call_site);

      fprintf(graphml_file, "\t\t\t<data key=\"d1\">\n");
      fprintf(graphml_file, "\t\t\t\t<y:PolyLineEdge>\n");
      fprintf(graphml_file, "\t\t\t\t\t<y:LineStyle type=\"line\" " "width=\"1.0\" color=\"#000000\"/>\n");
      // Label edge with count
      fprintf(graphml_file, "\t\t\t\t\t<y:EdgeLabel alignment=\"center\" backgroundColor=\"#FFFFFF\" modelName=\"centered\">%d</y:EdgeLabel>\n", child->count);
      fprintf(graphml_file, "\t\t\t\t\t<y:Arrows source=\"none\" target=\"standard\"/>\n");
      // Closing tags
      fprintf(graphml_file, "\t\t\t\t</y:PolyLineEdge>\n");
      fprintf(graphml_file, "\t\t\t</data>\n");
      fprintf(graphml_file, "\t\t</edge>\n");


      //push the child node to the queue
      enqueue(end, queue, child);
      end++;
      child = child->next;
    }
   }

  // Close file
  fprintf(graphml_file, "\n\t</graph>");
  fprintf(graphml_file, "\n</graphml>");
  fclose(graphml_file);
}


