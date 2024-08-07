#include <stdio.h>
#include "ddfa.h"

int mul(int a, int b) {
	return a*b;
}

data_map_t * const5;

void square(int num) {
	map_data(const5, MAP_TYPE_FUNCARGCOPY, "num", &num, sizeof(int), ACCESS_KIND_READ_ONLY, MEM_TYPE_HOSTMEM, 0);
    // Function to calculate the square of a number.
    int result = mul(num, num);
    //printf("%d\n", result);
}

int main() {
	int i;
	for (i=0; i<3; i++) {
		init_callarg_meta(NULL, MAP_TYPE_INIT_CONST, "5", NULL, sizeof(int), &square);
		square(5);
		init_callarg_meta(NULL, MAP_TYPE_INIT_CONST, "5", NULL, sizeof(int), &square);
		square(5);  // Output: 25
	}
    return 0;
}
