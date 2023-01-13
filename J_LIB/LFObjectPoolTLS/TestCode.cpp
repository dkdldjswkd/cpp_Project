#include <iostream>
#include <Windows.h>
#include <thread>
#include "CrashDump.h"
#include "LFObjectPoolTLS.h"
using namespace std;

#define MAX_ALLOC_SIZE 500000
struct MyStruct{
	long long a = 3;
	long long b = 4;
	long long c = 5;
};

MyStruct* g_arr[2][MAX_ALLOC_SIZE];

LFObjectPoolTLS<MyStruct> memTLS;

int arr_n = -1;
void f() {
	MyStruct** arr = g_arr[InterlockedIncrement((LONG*)&arr_n)];

	int alloc_cout = 0;
	int alloc_index = 0;
	int free_index = 0;

	while (true) {
		// 최대 alloc 가능 수
		int alloc_req = MAX_ALLOC_SIZE - alloc_cout;
		if (alloc_req != 0) {
			alloc_req = rand() % alloc_req; // 이번 프레임 Alloc 개수

			for (int i = 0; i < alloc_req; i++, alloc_index++) {
				if (alloc_index == MAX_ALLOC_SIZE)
					alloc_index = 0;

				arr[alloc_index] = memTLS.Alloc();
			}
			alloc_cout += alloc_req;
		}

		// 최대 free 가능 수
		int free_req = alloc_cout;
		if (free_req != 0) {
			free_req = rand() % free_req; // 랜덤 free 수

			for (int i = 0; i < free_req; i++, free_index++) {
				if (free_index == MAX_ALLOC_SIZE)
					free_index = 0;

				memTLS.Free(arr[free_index]);
			}
			alloc_cout -= free_req;
		}
	}
}

void m() {
	while (1) {
		Sleep(300);
		printf("ChunkCapacity : %d \n", memTLS.Get_ChunkCapacity());
		printf("ChunkUseCount : %d \n", memTLS.Get_ChunkUseCount());
		printf("UseCount : %d \n", memTLS.Get_UseCount());
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}
}

int main() {
	thread t[2];
	t[0] = thread(f);
	t[1] = thread(f);

	thread mo(m);
	if (mo.joinable())
		mo.join();
}