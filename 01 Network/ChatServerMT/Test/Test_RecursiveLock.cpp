#include <iostream>
#include <Windows.h>
#include <timeapi.h>
#include <thread>
#include "../../NetworkLib/RecursiveLock.h"
using namespace std;

RecursiveLock lock;
void f() {
	for (;;) {
		lock.Lock_Exclusive();
		lock.Lock_Exclusive();
		printf("( thread id : %d ) \n", GetCurrentThreadId());
		lock.Unlock_Exclusive();
		lock.Unlock_Exclusive();
	}
}

int main() {
	thread t1(f);
	thread t2(f);
	thread t3(f);
	Sleep(INFINITE);
}