#include <iostream>
#include <Windows.h>
#include <winnt.h>
#include <process.h>
using namespace std;

bool flag[2];
int turn = 2;

int cnt = 0;
int start = 0;

unsigned int f(void*) {
	for (;;) {
		if (start)
			break;
	}

	printf("start \n");

	for (int i = 0; i < 1000000; i++) {
		flag[0] = true;
		atomic_thread_fence(memory_order_seq_cst);
		turn = 1;
		while (flag[1] && turn == 1);

		cnt++;

		flag[0] = false;
	}

	printf("end \n");

	return 1;
}

unsigned int f2(void*) {
	for (;;) {
		if (start)
			break;
	}

	printf("start \n");

	for (int i = 0; i < 1000000; i++) {
		flag[1] = true;
		atomic_thread_fence(memory_order_seq_cst);
		turn = 0;
		while (flag[0] && turn == 0);

		cnt++;

		flag[1] = false;
	}

	printf("end \n");

	return 1;
}

int main() {
	unsigned int thread_id;
	unsigned int thread_id2;

	auto h = (HANDLE)_beginthreadex(NULL, 0, f, NULL, 0, &thread_id);
	auto h2 = (HANDLE)_beginthreadex(NULL, 0, f2, NULL, 0, &thread_id2);

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