#include <iostream>
#include <thread>
#include "../CrashDump.h"
#include "../LFStack.h"
using namespace std;

#define TEST_LOOP 100000
#define THREAD_NUM 3
#define PUSH_SIZE 100 // �����忡���� �ִ� ���� ��

thread t[THREAD_NUM];
LFStack<int> j_stack;
CrashDump dump;

void f() {
	int a;

	for (;;) {

		for (int j = 0; j < PUSH_SIZE; j++) {
			j_stack.Push(rand() % 1000);
		}

		for (int j = 0; j < PUSH_SIZE; j++) {
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

		if (n < 0 || n > THREAD_NUM * PUSH_SIZE) {
			CRASH();
		}
	}
}

thread monitor_t;

int LFStack_Test() {
	printf("TEST_LOOP : %d / THREAD_NUM : %d \n", TEST_LOOP, THREAD_NUM);

	monitor_t = thread(monitor_func);

	for (int i = 0; i < THREAD_NUM; i++) {
		t[i] = thread(f);
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		if (t[i].joinable())
			t[i].join();
	}

	printf("stack size : %d ", j_stack.GetUseCount());
	printf("�� \n");
}