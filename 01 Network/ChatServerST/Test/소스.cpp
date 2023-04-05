#include <iostream>
#include <queue>

int main() {
	int* p = (int*)malloc(sizeof(int));
	free(p);

	*p = 3;
	printf("%d \n", *p);
}