#include <iostream>
#include <thread>
#include "LockFreeStack.h"
#include "CrashDump.h"
using namespace std;

#define TEST_LOOP 1000000
#define THREAD_NUM 5

CrashDump dump;
thread t[THREAD_NUM];
LockFreeStack<int> j_stack;

void f() {
	int a;

	//for (int i = 0; i < TEST_LOOP; i++) {
	for (;;) {
		for (int i = 0; i < 5; i++) {
			j_stack.Push(rand() % 1000);
			j_stack.Pop(&a);
		}
	}
}

void monitor_func() {
	for (;;) {
		Sleep(1000);
		system("cls");
		int n = j_stack.GetUseCount();
		printf("stack size : %d ", n);

		if (n < 0 || n > 5) {
			CRASH();
		}
	}
}

thread monitor_t;

int main() {
	printf("TEST_LOOP : %d / THREAD_NUM : %d \n", TEST_LOOP, THREAD_NUM);

	monitor_t = thread(monitor_func);

	for (int i = 0; i < THREAD_NUM; i++) {
		t[i] = thread(f);
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		if (t[i].joinable())
			t[i].join();
	}

	if (monitor_t.joinable())
		monitor_t.join();

	printf("stack size : %d ", j_stack.GetUseCount());
	printf("³¡ \n");
}