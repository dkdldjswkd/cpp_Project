#include <iostream>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include "LFQueue.h"
#include "CrashDump.h"
using namespace std;

CrashDump dump;
thread t[THREAD_NUM];
LFQueue<int> Q;
int use_size = 0;

namespace J {
	bool shutdown = false;
}

BYTE flag = 0x90;
void f() {
	BYTE func_flag = InterlockedAdd((LONG*)&flag, 0x10);
	int a = 100;

	printf("flag : %02X \n", func_flag);
	for (int i = 0;; i++) {
		for (int j = 0; j < PUSH_LOOP; j++) {
			Q.Enqueue(a, func_flag);
		}
		if(i % 1000000 == 0)
			printf("Enqueue �Ϸ� \n");

		// CRASH, ť�� �̻��� ����
		use_size = Q.GetUseCount();
		if (use_size < -1 || use_size > MAX_NODE) CRASH();

		for (int j = 0; j < PUSH_LOOP; j++) {
			Q.Dequeue(&a, func_flag);
			if (a != 100)
				CRASH();
		}
		if (i % 1000000 == 0)
			printf("Dequeue �Ϸ� \n");

		// CRASH, ť�� �̻��� ����
		use_size = Q.GetUseCount();
		if (use_size < -1 || use_size > MAX_NODE) CRASH();

		// ������ ����
		if (J::shutdown) {
			printf("����");
			break;
		}
	}
}

void monitor_func() {
	int count = 0;

	for (;;) {
		Sleep(1000);
		printf("node count : %d \n", use_size);

		++count;
		if (count == 10)
			system("cls");

#pragma warning(suppress : 4996)
		if (kbhit()) {
#pragma warning(suppress : 4996)
			char key = getch();
			printf("%c \n", key);
			if (key == 's' || key == 'S') {
				J::shutdown = true;
				break;
			}
		}
	}
}

int main() {
	printf("������ �� Enqueue, Dequeue : %d / �ִ� ��� ���� : %d / THREAD_NUM : %d \n", PUSH_LOOP, MAX_NODE, THREAD_NUM);

	// ������ ����
	thread monitor_t(monitor_func);
	for (int i = 0; i < THREAD_NUM; i++) {
		t[i] = thread(f);
	}

	// ������ ����
	if (monitor_t.joinable())
		monitor_t.join();

	for (int i = 0; i < THREAD_NUM; i++) {
		if (t[i].joinable())
			t[i].join();
	}


	printf("�� \n");
}