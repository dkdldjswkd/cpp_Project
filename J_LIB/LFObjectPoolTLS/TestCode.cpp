#include <iostream>
#include <Windows.h>
#include <thread>
#include "CrashDump.h"
#include "LFObjectPoolTLS.h"
using namespace std;

#define TEST_LOOP 1000000
#define MAX_ALLOC 50
#define THREAD_NUM 3

thread t[THREAD_NUM];
LFObjectPoolTLS<int> memTLS;

void f() {
	int* p;

	for (;;) {
		p = memTLS.Alloc();
		memTLS.Free(p);
	}
}

int main() {
}