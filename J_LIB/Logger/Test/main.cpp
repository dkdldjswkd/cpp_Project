#include <Windows.h>
#include <thread>
#include <iostream>
#include <conio.h>
#include "Logger.h"
using namespace std;

void main() {
	LOG_INIT();

	for (;;) {
		Sleep(1000);
		printf("�α�... \n");
		LOG("battle", LOG_LEVEL_FATAL, "%d start battle  %s \n", 3, "asdf");
		LOG("battle", LOG_LEVEL_DEBUG, "%d asd \n", 3);

		if (_kbhit()) {
			auto c = _getch(); // ���� �� ����
			if (c == 's' || c == 'S') {
				printf("����... \n");
				LOG_RELEASE();
				break;
			}
		}
	}
}