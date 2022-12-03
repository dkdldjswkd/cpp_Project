#include <iostream>
#include <thread>
#include "LockFreeStack.h"
using namespace std;

thread t[THREAD_NUM];
LockFreeStack j_stack;

void f() {
	for (int i = 0; i < TEST_LOOP; i++)
		j_stack.Push(rand() % 1000);

	for (int i = 0; i < TEST_LOOP; i++)
		j_stack.Pop();
}

int main() {
	printf("½ÃÀÛ \n");

	for (int i = 0; i < THREAD_NUM; i++) {
		t[i] = thread(f);
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		if (t[i].joinable())
			t[i].join();
	}

	printf("³¡ \n");
}