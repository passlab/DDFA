#include "ddfa.h"

__thread data_map_t data_map_buffer[DATA_MAP_BUFFER_SIZE];
__thread int num_maps;
symbol_t sym_table[SYMBOL_TABLE_SIZE];

/**
 *
 * @param index: the index of the argument in a function call
 * @param count: how many times this map is requested, NOTE: not all request will create a map since it depends on the traceKind

 */
data_map_t *map_data(data_map_t *src, map_type_t mapType, trace_kind_t traceKind, char *symbol, void *addr, size_t size, access_kind_t accessKind, mem_type_t memType, int devId, int index, int count) {
  //Find the key for this mapping request

  //Check whether this map is created before or not
  //If not created before, create and init it
  //If created, check the traceKind to see whether a new map should be created. With full tracing of every call, the count should be different. 
  //So a data map can be uniquely identified as key:index:count

  
  data_map_t *map = &data_map_buffer[num_maps];
  num_maps++;
  map->symbol = symbol;
  map->addr = addr;
  map->size = size;
  map->accessKind = accessKind;
  map->devId = devId;
  map->memType = memType;

  map->src = src;
  map->mapType = mapType;
  map->traceKind = traceKind;
  // callpath_key_t callpath_key;

  //attach the call path to the call graph
  //attach_callpath(root, 2);

  // attach the map to the call graph
  // Assume top is the function that makes this map
  map->next = top->data_maps;
  top->data_maps = map; // XXX: Is there data racing in this situation??
}

/*
data_map_t * init_callarg_meta(data_map_t * src, map_type_t mapType, trace_kind_t traceKind, char * symbol, void * addr, size_t size, void * func) {
  data_map_t * temp = map_data(src, mapType, symbol, addr, size, ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);

  top->next_callarg_meta = temp;
  temp->argnext = NULL;
}

data_map_t * add_callarg_meta(data_map_t * src, map_type_t mapType, char * symbol, void * addr, size_t size, void * func) {
  data_map_t * temp = map_data(src, mapType, symbol, addr, size, ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);

  temp->argnext = top->next_callarg_meta;
  top->next_callarg_meta = temp;
}
*/
