#include <iostream>
#include <Windows.h>
#include "../../../00 lib_jy/ThreadCpuMonitor.h"
#include "../../../00 lib_jy/SegmentCpuMonitor.h"
#pragma comment (lib, "Winmm.lib")
using namespace std;

void BusyWait(DWORD sleep_ms) {
	DWORD startTime = timeGetTime();
	while (timeGetTime() - startTime < sleep_ms) {}
}

int main() {
	SegmentCpuMonitor monitor;
	for (int i = 0;; i++) {
		// 측정시작
		monitor.SetNano();
		monitor.Begin();

		// 코드 실행 부
		int aaaa = 3;
		printf("");

		// 측정종료
		static double a = 0, b = 0, c = 0, d = 0;
		a += monitor.End();
		b += monitor.GetTotalTime();
		c += monitor.GetUserTime();
		d += monitor.GetKernelTime();

		// Print
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
		printf("Segment    Time : %lf       \n", a);
		printf("Total  Cpu Time : %lf       \n", b);
		printf("User   Cpu Time : %lf       \n", c);
		printf("Kernel Cpu Time : %lf       \n", d);
	}
}