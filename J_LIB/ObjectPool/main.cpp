#include <iostream>
#include <Windows.h>
#include <thread>
#include "ObjectPool.h"
using namespace std;

#define TEST_LOOP 100000
#define THREAD_NUM 5

struct Node {
	Node() {
		data = 1;
	}

	int data;
};

J_LIB::ObjectPool<Node> nodePool(0, true);

void func() {
	printf("Ω√¿€ \n");

	Node* node[TEST_LOOP];

	//for (int i = 0; i < TEST_LOOP / 100; i++) {
	for (;;) {
		int rand_n = rand();

		for (int j = 0; j < 100; j++) {
			node[j] = nodePool.Alloc();
			if (node[j]->data != 1)
				CRASH();
			node[j]->data = rand_n;
		}

		for (int j = 0; j < 100; j++) {
			if (node[j]->data != rand_n)
				CRASH();
			nodePool.Free(node[j]);
		}
	}

	printf("≥° \n");
}

void monitor_func() {
	for (;;) {
		Sleep(1000);
		system("cls");
		int a = nodePool.GetUseCount();
		int b = nodePool.GetCapacityCount();
		printf("GetUseCount : %d \n", a);
		printf("GetCapacityCount : %d \n", b);

		if(a < 0 || a > b)
			CRASH();
		if (b > 100 * THREAD_NUM)
			CRASH();
	}
}

int node_cout = 0;

int main() {
	thread t[THREAD_NUM];

	for (int i = 0; i < THREAD_NUM; i++) 
		t[i] = thread(func);

	thread monitor_t = thread(monitor_func);

	for (int i = 0; i < THREAD_NUM; i++)
		if (t[i].joinable())
			t[i].join();
}