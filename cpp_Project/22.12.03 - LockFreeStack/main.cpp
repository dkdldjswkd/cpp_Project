#include <iostream>
#include <thread>
#include "LockFreeStack.h"
#include "CrashDump.h"
using namespace std;

CrashDump dump;
thread t[THREAD_NUM];
LockFreeStack j_stack;

BYTE flag = 0x90;
void f() {
	BYTE func_flag = InterlockedAdd((LONG*)&flag, 0x10);

	printf("flag : %02X \n", func_flag);

	for (int i = 0; i < TEST_LOOP; i++)
		j_stack.Push(func_flag);

	for (int i = 0; i < TEST_LOOP; i++)
		j_stack.Pop(func_flag);

	for (int i = 0; i < TEST_LOOP; i++)
		j_stack.Push(func_flag);

	for (int i = 0; i < TEST_LOOP; i++)
		j_stack.Pop(func_flag);
}

int main() {
	printf("TEST_LOOP : %d / THREAD_NUM : %d \n", TEST_LOOP, THREAD_NUM);

	for (int i = 0; i < THREAD_NUM; i++) {
		t[i] = thread(f);
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		if (t[i].joinable())
			t[i].join();
	}

	printf("³¡ \n");
}