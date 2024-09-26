#include "ddfa.h"
#include <stdio.h>

int mul(int a, int b) { 
  //data_map_t * aMap = map_data(NULL, "a", &a, sizeof(int), 0, 0, 
  //   ACCESS_KIND_UNKNOWN, MAP_TYPE_COPY, MAP_CLASS_FUNCPARA, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);
  //func_para_map(&mul, aMap, 0);
  //data_map_t * bMap = map_data(NULL, "b", &b, sizeof(int), 0, 0, 
  //   ACCESS_KIND_UNKNOWN, MAP_TYPE_COPY, MAP_CLASS_FUNCPARA, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH, 0);
  //func_para_map(&mul, bMap, 1);

  int amulb = a*b;

  //data_map_t * amulbMap = map_data(NULL, "amulb", &amulb, sizeof(int), 0, 0, 
  //   ACCESS_KIND_UNKNOWN, MAP_TYPE_INIT_VAR, MAP_CLASS_VAR, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,  0);
  //func_return_map(&mul, amulbMap);
  return amulb; 
}

data_map_t *const5;

void square(int num) {
  data_info_t * num_info = init_data_info("num", &num, sizeof(int), 0, DATA_CLASS_PVAR, ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH, 0);
  map_funccall_argu(&square, num_info, 0);
  end_map_funccall_argu ();



  for (int a = 0; a < 10; a++) {
    //func_arg_map(&mul, numMap, 0);
    //func_arg_map(&mul, numMap, 1);
    mul(num, num);
  }
  //func_arg_map(&mul, numMap, 0);
  //func_arg_map(&mul, numMap, 0);
  int result = mul(num, num);
  //data_map_t * resultMap = map_data(NULL, "result", &result, sizeof(int), 0, 0, 
   //  ACCESS_KIND_UNKNOWN, MAP_TYPE_INIT_VAR, MAP_CLASS_VAR, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH,   0);
  //func_collect_return(&mul, resultMap);
  // printf("%d\n", result);
}

int main() {
  int i;
//#pragma omp parallel for
  for (i = 0; i < 12; i++) {
    //create a data map for constant 5, since it is constant, there is no src for this data map
    //this will be manually instrumented function call
    data_info_t * const5_info = init_data_info(NULL, NULL, sizeof(int), 0, DATA_CLASS_CONST, 
        ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, TRACE_KIND_PER_CALLPATH, 0); //init a data_info object for const 5
    add_func_argu(&square, const5_info, 0); //attach the data_info object to the runtime for the arg of the upcoming square(5) call
    square(5);
    if (i % 2 == 0) {
      square(6);
    }
    square(5); // Output: 25
  }
  return 0;
}
