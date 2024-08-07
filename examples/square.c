#include "ddfa.h"
#include <stdio.h>

int mul(int a, int b) { return a * b; }

data_map_t *const5;

void square(int num) {
  for (int a = 0; a < 10; a++) {
    mul(num, num);
  }
  // map_data(const5, MAP_TYPE_FUNCARGCOPY, "num", &num, sizeof(int),
  // ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);
  // Function to calculate the square of a number.
  int result = mul(num, num);
  // printf("%d\n", result);
}

int main() {
  int i;
#pragma omp parallel for
  for (i = 0; i < 12; i++) {
    // printf("\nLOOP #%d\n", i);
    //init_callarg_meta(NULL, MAP_TYPE_INIT_CONST, "5", NULL, sizeof(int), &square);
    square(5);
    if (i % 2 == 0) {
      square(6);
    }
    // const5 = map_data(NULL, MAP_TYPE_INIT_CONST, "5", NULL, sizeof(int),
    // ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);
    //init_callarg_meta(NULL, MAP_TYPE_INIT_CONST, "5", NULL, sizeof(int), &square);
    square(5); // Output: 25
  }
  return 0;
}
