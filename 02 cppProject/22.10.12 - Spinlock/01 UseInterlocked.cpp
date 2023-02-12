#include <iostream>
#include <Windows.h>
#include <winnt.h>
#include <process.h>
using namespace std;

long lock = 0;

void JLock() {
	for (;;) {
		// 잠겨있지 않으면 실행 (lock == false)
		if (false == InterlockedExchange(&lock, 1)) {
			return;
		}
	}
}

void JUnlock() {
	//InterlockedExchange(&lock, 0);
	lock = 0;
}

int cnt = 0;
int start = 0;

unsigned int f(void*) {
	for (;;) {
		if (start)
			break;
	}

	printf("start \n");
	for (int i = 0; i < 1000000; i++) {
		JLock();
		cnt++;
		JUnlock();
	}
	printf("end \n");

	return 1;
}

int main() {
	unsigned int thread_id;
	unsigned int thread_id2;

	auto h =  (HANDLE)_beginthreadex(NULL, 0, f, NULL, 0, &thread_id);
	auto h2 =  (HANDLE)_beginthreadex(NULL, 0, f, NULL, 0, &thread_id2);

	if (h == INVALID_HANDLE_VALUE || h == 0)
		return 0;
	if (h2 == INVALID_HANDLE_VALUE || h2 == 0)
		return 0;

	start = 1;

	WaitForSingleObject(h, INFINITE);
	WaitForSingleObject(h2, INFINITE);
	cout << cnt << endl;

	return 1;
}