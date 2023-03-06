#include <iostream>
#include <Windows.h>
#include "../../../00 lib_jy/ThreadCpuMonitorTLS.h"
#pragma comment (lib, "Winmm.lib")
using namespace std;

ThreadCpuMonitorTLS monitor;

void BusyWait(DWORD sleep_ms) {
	DWORD startTime = timeGetTime();
	while (timeGetTime() - startTime < sleep_ms) {}
}

int main() {
	monitor.UpdateCpuUsage(); 
	//monitor.UpdateCpuUsage(); 
	/*for (int i = 0; i <= 1000000; i++) {
		Sleep(0);
		int a = i % (i / 2 + 1);
	}*/
	BusyWait(1000);
	monitor.UpdateCpuUsage(); 
	cout << monitor.GetTotalCpuUsage() << endl;
	cout << monitor.GetKernelCpuUsage() << endl;
	cout << monitor.GetUserCpuUsage() << endl;
}