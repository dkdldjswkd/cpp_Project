#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <thread>
using namespace std;

void f();

SRWLOCK lock;
int main() {
	InitializeSRWLock(&lock);
	thread t(f);

	Sleep(INFINITE);
}

void f() {
	AcquireSRWLockExclusive(&lock);
	int a =1;
	a++;
	ReleaseSRWLockExclusive(&lock);
}