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

		new char[1024 * 1024 * 100]; // 100 mb �Ҵ�

		cout << "���μ��� �����Ҵ� �޸�		: " << counter.GetUserMem() << endl;
		cout << "���μ��� �������� �޸�		: " << counter.GetProcessNonMem() << endl;
		cout << "�ý��ۿ��� �Ҵ簡���� �޸� : " << counter.GetAvailMem() << endl;
		cout << "�ý��� �������� �޸�		: " << counter.GetSysNonMem() << endl;
		cout << "�ʴ� Recv Bytes		: " << counter.GetRecvBytes() << endl;
		cout << "�ʴ� Send Bytes		: " << counter.GetSendBytes() << endl;
	}
}