#include <iostream>
#include <thread>
#include "LFQueue.h"
#include "CrashDump.h"
using namespace std;

MemoryNote note(8);

CrashDump dump;
thread t[THREAD_NUM];
LFQueue<int> Q;

BYTE flag = 0x90;
void f() {
	BYTE func_flag = InterlockedAdd((LONG*)&flag, 0x10);
	int a = 0;

	printf("flag : %02X \n", func_flag);
	for (int i = 0; i < TEST_LOOP; i++) {
		for (int j = 0; j < PUSH_LOOP; j++)
			Q.Enqueue(++a, func_flag);

		for (int j = 0; j < PUSH_LOOP; j++)
			Q.Dequeue(&a, func_flag);
	}
}

int main() {
	printf("스레드 당 Enqueue, Dequeue : %d / 최대 노드 개수 : %d / THREAD_NUM : %d \n", PUSH_LOOP, MAX_NODE, THREAD_NUM);

	for (int i = 0; i < THREAD_NUM; i++) {
		t[i] = thread(f);
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		if (t[i].joinable())
			t[i].join();
	}

	printf("끝 \n");
}