#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <thread>
#include <stack>
#include <mutex>
#include "LFObjectPoolTLS.h"
using namespace std;
using namespace J_LIB;

struct ST {
	bool flag;
};
LFObjectPoolTLS<ST> pool;

void f() {
	auto p = pool.Alloc();
	pool.Free(p);
}

void test() {
	for (int i=0;;i++)
		f();
}

void monitor() {
	for (;;) {
		Sleep(100);
		printf("UseCount : %d \n\n", pool.Get_UseCount());
		printf("ChunkCapacity : %d \n", pool.Get_ChunkCapacity());
		printf("ChunkUseCount : %d \n", pool.Get_ChunkUseCount());
		printf("\n\n\n\n\n\n\n\n\n\n");
		printf("\n\n\n\n\n");
		//system("cls");
	}
}

int main() {
	thread t1(test);
	thread t2(test);
	thread t3(monitor);
	Sleep(INFINITE);
}