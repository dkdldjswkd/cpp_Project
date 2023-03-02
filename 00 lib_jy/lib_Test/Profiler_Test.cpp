#include <iostream>
#include <Windows.h>
#include <thread>
#include <conio.h>
#include "../Profiler.h"
using namespace std;

#define RAND_N (rand() +1)

bool quit = false;

void f1() {
	while (!quit) {
		PRO_BEGIN("f1");
		for (int i = 0; i < 1000000; i++) {
			int a = 3000;
			int b = a % RAND_N % RAND_N % RAND_N % RAND_N % RAND_N;
		}
		PRO_END("f1");
		//printf("f1() \n");
	}
}

void f2() {
	while (!quit) {
		PRO_BEGIN("f2");
		for (int i = 0; i < 1000000; i++) {
			int a = 3000;
			int b = a * RAND_N * RAND_N * RAND_N * RAND_N * RAND_N;
		}
		PRO_END("f2");
		//printf("f2() \n");
	}
}

void quit_f() {
	while (false == quit) {
		char ch = _getch();
		switch (ch)
		{
		case 'q':
		case 'Q':
			quit = true;
			break;

		case 'r':
		case 'R':
			PRO_RESET();
			break;

		case 'f':
		case 'F':
			PRO_FILEOUT();
			printf("PRO_FILEOUT \n");
			break;

		}
	}
}

int Profiler_Test() {
	thread t[3];
	t[0] = thread(f1);
	t[1] = thread(f1);
	t[2] = thread(f2);

	thread quit_t(quit_f);
	quit_t.join();

	for (int i = 0; i < 3; i++) {
		if (t[i].joinable())
			t[i].join();
	}
}