#include "ddfa.h"

data_map_t *map_data(data_map_t *src, map_type_t mapType, char *symbol,
                     void *addr, size_t size, access_kind_t accessKind,
                     mem_type_t memType, int devId) {
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
  // callpath_key_t callpath_key;

  attach_callpath(root, 2);
  // attach the call path to the call graph

  // attach the map to the call graph
  // Assume top is the function that makes this map
  map->next = top->data_maps;
  top->data_maps = map; // XXX: Is there data racing in this situation??
}

data_map_t * init_callarg_meta(data_map_t * src, map_type_t mapType, map_kind_t mapKind, char * symbol, void * addr, size_t size, void * func) {
  data_map_t * temp = map_data(src, mapType, symbol, addr, size, ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);

  top->next_callarg_meta = temp;
  temp->argnext = NULL;
}

data_map_t * add_callarg_meta(data_map_t * src, map_type_t mapType, char * symbol, void * addr, size_t size, void * func) {
  data_map_t * temp = map_data(src, mapType, symbol, addr, size, ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);

  temp->argnext = top->next_callarg_meta;
  top->next_callarg_meta = temp;
}
