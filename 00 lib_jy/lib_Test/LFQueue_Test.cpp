#include <iostream>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include "../LFQueue.h"
#include "../CrashDump.h"
using namespace std;

#define THREAD_NUM	2
#define TEST_LOOP	100
#define PUSH_LOOP	2
#define MAX_NODE	THREAD_NUM * PUSH_LOOP

CrashDump dump;
thread t[THREAD_NUM];
LFQueue<int> Q;
int use_size = 0;
bool _shutdown = false;

void f() {
	int a = 100;

	for (int i = 0;; i++) {
		for (int j = 0; j < PUSH_LOOP; j++) {
			Q.Enqueue(a);

			// ������ ���� ����
			auto count = Q.GetUseCount();
			if (MAX_NODE < count || count < 0) CRASH();
		}

		// ������ ����
		if (_shutdown) {
			printf("�� �ٿ� \n");
			break;
		}

		for (int j = 0; j < PUSH_LOOP; j++) {
			Q.Dequeue(&a);

			// ������ ���� ����
			auto count = Q.GetUseCount();
			if (MAX_NODE < count || count < 0 || a != 100) CRASH();
		}
	}
}

void monitor_func() {
	int count = 0;

	for (;;) {
		Sleep(300);
		printf("node count : %d // Q,q �Է½� ���� \n", Q.GetUseCount());

		++count;
		if (count == 100) {
			system("cls");
			count = 0;
		}

#pragma warning(suppress : 4996)
		if (kbhit()) {
#pragma warning(suppress : 4996)
			char key = getch();
			printf("%c \n", key);
			if (key == 'q' || key == 'Q') {
				_shutdown = true;
				return;
			}
		}
	}
}

int LFQueue_Test() {
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


	if (Q.GetUseCount() != MAX_NODE) {
		printf("����, ��� ���� �ȸ��� ũ����!! \n");
		CRASH();
	}
	printf("���� ����. \n");
}