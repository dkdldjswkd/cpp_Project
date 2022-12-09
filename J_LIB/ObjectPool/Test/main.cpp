#include <iostream>
#include <Windows.h>
#include <thread>
#include "ObjectPool.h"
using namespace std;

#define TEST_LOOP 10000
#define THREAD_NUM 5

struct Node {
    int a;
    int b;
};

J_LIB::ObjectPool<Node> nodePool(100000, true);

void func() {
	printf("Ω√¿€ \n");

	for (int i = 0; i < TEST_LOOP; i++) {
		Node* p = nodePool.Alloc();
		//nodePool.Free(p);
	}

	printf("≥° \n");
}

int node_cout = 0;

int main() {
	thread t[THREAD_NUM];

	for (int i = 0; i < THREAD_NUM; i++) 
		t[i] = thread(func);

	for (int i = 0; i < THREAD_NUM; i++)
		if (t[i].joinable())
			t[i].join();

	printf("GetUseCount : %d \n", nodePool.GetUseCount());
	printf("GetCapacityCount : %d \n", nodePool.GetCapacityCount());
}