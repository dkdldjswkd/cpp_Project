#include <iostream>
#include <queue>

int main() {
	int* p = (int*)malloc(sizeof(int));
	*p = 3;
	free(p);

	printf("%d \n", *p);
}