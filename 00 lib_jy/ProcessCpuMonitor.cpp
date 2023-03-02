#include "ProcessCpuMonitor.h"
#include <iostream>

// 확인 프로세스 핸들
ProcessCpuMonitor::ProcessCpuMonitor(HANDLE hProcess){
	if (hProcess == INVALID_HANDLE_VALUE) {
		h_process = GetCurrentProcess();
	}

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	NumOfCore = SystemInfo.dwNumberOfProcessors; // 논리 코어 개수
	UpdateCpuUsage();
}

// CPU 사용률 갱신 (500 ~ 1000 ms 주기 호출)
void ProcessCpuMonitor::UpdateCpuUsage() {
	ULARGE_INTEGER none; // 미사용
	ULARGE_INTEGER curTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	// 현재시간 & 누적 코어 사용시간
	GetSystemTimeAsFileTime((LPFILETIME)&curTime);
	GetProcessTimes(h_process, (LPFILETIME)&none, (LPFILETIME)&none, (LPFILETIME)&curkernelTime, (LPFILETIME)&curUserTime);

	// 코어 사용 시간 (해당 프로세스의)
	ULONGLONG deltaTime			= curTime.QuadPart		 - prevTime.QuadPart;
	ULONGLONG deltaUserTime		= curUserTime.QuadPart	 - prevUserTime.QuadPart;
	ULONGLONG deltaKernelTime	= curkernelTime.QuadPart - prevKernelTime.QuadPart;
	ULONGLONG totalTime = deltaKernelTime + deltaUserTime;

	// 코어 사용 시간 백분률
	coreTotal = (float)(totalTime / (double)NumOfCore / (double)deltaTime * 100.0f);
	coreUser = (float)(deltaUserTime / (double)NumOfCore / (double)deltaTime * 100.0f);
	coreKernel = (float)(deltaKernelTime / (double)NumOfCore / (double)deltaTime * 100.0f);

	prevTime = curTime;
	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
}

float ProcessCpuMonitor::GetTotalCpuUsage() {
	return coreTotal;
}

float ProcessCpuMonitor::GetUserCpuUsage() {
	return coreUser;
}

float ProcessCpuMonitor::GetKernelCpuUsage() {
	return coreKernel;
}