#include <iostream>
#include <Windows.h>
#include "../ProcessCpuMonitor.h"
#include "../MachineCpuMonitor.h"
#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

void CpuMonitor_Test() {
	thread t1([] {int a = 0; for (;;) { a++; printf(""); printf(""); printf(""); printf(""); printf(""); printf(""); } });
	thread t2([] {int a = 0; for (;;) { a++; printf(""); printf(""); printf(""); printf(""); printf(""); printf(""); } });
	thread t3([] {int a = 0; for (;;) { a++; printf(""); printf(""); printf(""); printf(""); printf(""); printf(""); } });
	thread t4([] {int a = 0; for (;;) { a++; printf(""); printf(""); printf(""); printf(""); printf(""); printf(""); } });

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	MachineCpuMonitor machineMonitor; // CPUTime(hProcess)
	ProcessCpuMonitor processMonitor; // CPUTime(hProcess)
	while (1) {
		Sleep(1000);
		machineMonitor.UpdateCpuUsage();
		processMonitor.UpdateCpuUsage();
		SetConsoleCursorPosition(hConsole, { 0, 0 });

		printf("머신     cpu     사용량 : %f\n", machineMonitor.GetTotalCpuUsage());
		printf("프로세스 cpu 총   사용량 : %f\n", processMonitor.GetTotalCpuUsage());
		printf("프로세스 cpu 커널 사용량 : %f\n", processMonitor.GetKernelCpuUsage());
		printf("프로세스 cpu 유저 사용량 : %f\n", processMonitor.GetUserCpuUsage());
	}
}