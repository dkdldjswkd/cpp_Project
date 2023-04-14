#include "../PerformanceCounter.h"
#include <iostream>
#include <string>
using namespace std;

void PerformanceCounter_Test() {
	PerformanceCounter counter;
	while (1) {
		Sleep(500);
		system("cls");
		counter.Update();
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });

		new char[1024 * 1024 * 100]; // 100 mb 할당

		cout << "프로세스 유저할당 메모리		: " << counter.GetUserMemB() << endl;
		cout << "프로세스 논페이지 메모리		: " << counter.GetProcessNonMemB() << endl;
		cout << "시스템에서 할당가능한 메모리 : " << counter.GetAvailMemMB() << endl;
		cout << "시스템 논페이지 메모리		: " << counter.GetSysNonMemB() << endl;
		cout << "초당 Recv Bytes		: " << counter.GetRecvBytes() << endl;
		cout << "초당 Send Bytes		: " << counter.GetSendBytes() << endl;
	}
}