#include <stdio.h>

int mul(int a, int b) {
	return a*b;
}

void square(int num) {
    // Function to calculate the square of a number.
    int result = mul(num, num);
    printf("%d\n", result);
}

int main() {
	int i;
	for (i=0; i<3; i++) square(5);  // Output: 25
    return 0;
}
