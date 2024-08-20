#include "ddfa.h"
#include <stdio.h>

int mul(int a, int b) { return a * b; }

data_map_t *const5;

void square(int num) {
  //create a data map for parameter num. The ddfa lib should link the map from the caller (5) to this num, 
  //the caller map for 5 is the src for the map for num
  //this will be manually instrumented function call
  for (int a = 0; a < 10; a++) {
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
    square(5);
    if (i % 2 == 0) {
      square(6);
    }
    square(5); // Output: 25
  }
  return 0;
}
