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

DWORD64 delta;
DWORD64 loop;
DWORD64 cpuUsage;
double user;
double kernel;

SegmentCpuMonitor monitor;
int main() {
	monitor.SetMilli();
	monitor.Begin();
	BusyWait(300);
	Sleep(300);
	cout << monitor.End() << endl;
	cout << monitor.GetTotalTime() << endl;
	cout << monitor.GetUserTime() << endl;
	cout << monitor.GetKernelTime() << endl;

	//// 측정 시작
	//codeMonitor.Init();

	//int a = 1;
	//printf("");

	//// cpu 사용 률 측정
	//codeMonitor.UpdateCpuUsage();
	//loop++;
	//delta += codeMonitor.GetDeltaTime();
	//user += codeMonitor.GetDeltaUserTime();
	//kernel += codeMonitor.GetDeltaKernelTime();
	//cpuUsage += codeMonitor.GetDeltaTotalTime();
	//if (loop == 10000) {
	//	cout << "측정 시간   : " << delta << endl;
	//	cout << "CPU 사용시간  : " << cpuUsage << endl;
	//	cout << "CPU 사용률  : " << (cpuUsage / (delta)+1) / 100 << endl;
	//	cout << "유저 사용률 : " << user / (delta + 1) << endl;
	//	cout << "커널 사용률 : " << kernel / (delta + 1) << endl;
	//	loop = 0;
	//	delta = 0;
	//	cpuUsage = 0;
	//	user = 0;
	//	kernel = 0;
	//}
}

// 10000