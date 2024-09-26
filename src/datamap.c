#include "ddfa.h"

__thread data_map_t data_info_buffer[DATA_MAP_BUFFER_SIZE];
__thread data_map_t data_map_buffer[DATA_MAP_BUFFER_SIZE];
__thread int num_maps;
__thread int num_data_infos;
symbol_t sym_table[SYMBOL_TABLE_SIZE];

__thread struct data_map_attr_link_head {
  data_map_t * next;
  data_map_t * prev;
} data_map_attr_link_heads[64]; //heads of data maps of each categories.

void link_data_map_attr(data_map_t * map) {
  int i;
  for (i=0; i<MAPATTR_num_attrs; i++) {
    int attr = map->map_attrs[i].attr;
    struct data_map_attr_link_head  * head = &data_map_attr_link_heads[attr]; //Find the head of the maps that have the same attr

	  if (head->next == NULL) {
		  map->map_attrs[i].next = map;
		  map->map_attrs[i].prev = map;
		  head->next = head->prev = map;
	  } else { //append to the double link list
		  data_map_t * last = head->prev;
		  last->next = map;
		  map->map_attrs[i].prev = last;

		  map->map_attrs[i].next = head;
		  head->prev = map;
	  }
  }
}

/**
 * Create a meta-data object that will be used for mapping
 * @param index: the index of the argument in a function call
 * @param count: how many times this map is requested, NOTE: not all request will create a map since it depends on the traceKind
 */
data_info_t *init_data_info(char *symbol, void *addr, size_t size, int devId, data_class_t dataClass,
     access_kind_t accessKind, mem_type_t memType, trace_kind_t traceKind, int count) {
  //Find the key for this mapping request

  //Check whether this map is created before or not
  //If not created before, create and init it
  //If created, check the traceKind to see whether a new map should be created. With full tracing of every call, the count should be different. 
  //So a data map can be uniquely identified as key:index:count

  data_info_t *info = &data_info_buffer[num_maps];
  num_data_infos++;
  info->symbol = symbol;
  info->addr = addr;
  info->size = size;
  info->devId = devId;
  info->data_attrs[DATAATTR_data_class].attr = dataClass;
  info->data_attrs[DATAATTR_access_kind].attr = accessKind;
  info->data_attrs[DATAATTR_mem_type].attr = memType;
  info->traceKind = traceKind;
  info->count = count;

  //link_data_map_attr(map);

  // callpath_key_t callpath_key;

  //attach the call path to the call graph
  //attach_callpath(root, 2);

  // attach the map to the call graph
  // Assume top is the function that makes this map
  info->next = top->data_infos;
  top->data_infos = info; // XXX: Is there data racing in this situation??

  info->next2 = NULL;
  info->reference = NULL;
}

//* this has to be called right before fun call
void add_func_argu(void * func, data_info_t * info, int index) {
  info->reference = func;
  info->index = index;
  
  info->next2 = top->temp_info;
  top->temp_info = info;
}

/**
 * Link two m
 */
data_map_t * map_data(data_map_t * dest, data_map_t *src, map_type_t mapType, map_class_t mapClass, trace_kind_t traceKind, int count) {
  data_map_t *map = &data_map_buffer[num_maps];
  num_maps++;
  map->src = src;
  map->dest = dest;
  map->map_attrs[MAPATTR_map_type].attr = mapType;
  map->map_attrs[MAPATTR_map_class].attr = mapClass;
  map->traceKind = traceKind;
  map->count = count;

  map->next = top->data_maps;
  top->data_maps = map; //likely data race
}

/**
 * This func search the parent call node to find the argument data_info 
 * using index and then map the para data_info with the arg data_info
 * This func should be called at the beginning of a function, but after the data_info
 * for parameters is created
 */
data_map_t * map_funccall_argu(void * func, data_info_t * para, int index) {
  data_info_t *arg_info = top->parent->temp_info;
  while (arg_info != NULL) {
    if (arg_info->reference == func && arg_info->index == index) { //find the argument
      data_map_t * map = map_data(para, arg_info, MAP_TYPE_COPY, MAP_CLASS_FUNCPARA, arg_info->traceKind, arg_info->count);
      printf("arg-para mapped");

      //add to the arg_maps of the call
      map->next2 = top->arg_maps;
      top->arg_maps = map;
      return map;
    } else arg_info = arg_info->next2;
  }

  return NULL;

}

/**
 * we need a function to set NULL of temp_info of the parent call_t node so it will be properly used for later one
 * called within a callee
 */
void end_map_funccall_argu () {
  top->arg_infos = top->parent->temp_info;  //Just to keep it handy so far, and not sure it will be useful for the future. 
  top->parent->temp_info = NULL;

}
