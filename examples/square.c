#include "ddfa.h"
#include <stdio.h>

int mul(int a, int b) { 
  return a * b; 
}

data_map_t *const5;

void square(int num) {
  data_map_t * numMap = map_data(NULL, "num", &num, sizeof(int), 0, 0, 
     ACCESS_KIND_UNKNOWN, MAP_TYPE_FUNCARGCOPY, TRACE_KIND_PER_CALLPATH, MEM_TYPE_HOSTMEM,  0);
  //create a data map for parameter num. The ddfa lib should link the map from the caller (5) to this num, 
  //the caller map for 5 is the src for the map for num
  //this will be manually instrumented function call

  for (int a = 0; a < 10; a++) {
    func_arg_map(&mul, numMap, 0);
    func_arg_map(&mul, numMap, 1);
    mul(num, num);
  }
  int result = mul(num, num);
  // printf("%d\n", result);
}

int main() {
  int i;
#pragma omp parallel for
  for (i = 0; i < 12; i++) {
    //create a data map for constant 5, since it is constant, there is no src for this data map
    //this will be manually instrumented function call
    data_map_t * const5Map = map_data(NULL, "", NULL, sizeof(int), 0, 0, 
     ACCESS_KIND_READ_ONLY, MAP_TYPE_INIT_CONST, TRACE_KIND_PER_CALLPATH, MEM_TYPE_HOSTMEM,  i) ;
    func_arg_map(&square, const5Map, 0);
    square(5);
    if (i % 2 == 0) {
      square(6);
    }
    square(5); // Output: 25
  }
  return 0;
}
