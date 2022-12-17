#include <iostream>
#include <Windows.h>
#include <thread>
#include "CrashDump.h"
#include "LFObjectPool.h"
using namespace std;

#define TEST_LOOP 1000000
#define MAX_ALLOC 50
#define THREAD_NUM 3

bool _shutdown;
CrashDump dump;

struct Node {
	Node() {}
	int data = 1;
};

J_LIB::LFObjectPool<Node> nodePool(0, true);

void func() {
	printf("Ω√¿€ \n");

	Node* node[MAX_ALLOC];

	for (;;) {
		int rand_n = rand();

		for (int j = 0; j < MAX_ALLOC; j++) {
			node[j] = nodePool.Alloc();
			if (node[j]->data != 1) CRASH();
			node[j]->data = rand_n;
		}

		for (int j = 0; j < MAX_ALLOC; j++) {
			if (node[j]->data != rand_n) CRASH();
			nodePool.Free(node[j]);
		}
	}

	printf("≥° \n");
}

void monitor_func() {
	for (;!_shutdown;) {
		Sleep(1000);
		system("cls");
		int a = nodePool.GetUseCount();
		int b = nodePool.GetCapacityCount();
		printf("GetUseCount : %d \n", a);
		printf("GetCapacityCount : %d \n", b);

		if (b > MAX_ALLOC * THREAD_NUM) CRASH();
		if (a < 0 || a > b)				CRASH();
	}
}

int node_cout = 0;

int main() {
	thread t[THREAD_NUM];

	for (int i = 0; i < THREAD_NUM; i++) 
		t[i] = thread(func);

	thread monitor_t = thread(monitor_func);

	for (int i = 0; i < THREAD_NUM; i++)
			t[i].join();
	_shutdown = true;

	monitor_t.join();
}