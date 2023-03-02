#include "MachineCpuMonitor.h"
#include <iostream>

// 확인 프로세스 핸들
MachineCpuMonitor::MachineCpuMonitor() {
	// 프로세스 실행률 == (사용시간 / 논리코어 수)
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	NumOfCore = SystemInfo.dwNumberOfProcessors; // 논리 코어 개수
	UpdateCpuUsage();
}

// CPU 사용률 갱신 (500 ~ 1000 ms 주기 호출)
void MachineCpuMonitor::UpdateCpuUsage() {
	ULONGLONG deltaTime;
	ULARGE_INTEGER curIdleTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	// 누적 코어 사용 시간 (커널, 유저, idle)
	if (GetSystemTimes((PFILETIME)&curIdleTime, (PFILETIME)&curkernelTime, (PFILETIME)&curUserTime) == false) {
		return;
	}

	// 코어 사용 시간 (델타, 현재 - 마지막 호출)
	ULONGLONG deltaIdleTime = curIdleTime.QuadPart - prevIdleTime.QuadPart;
	ULONGLONG deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	ULONGLONG deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	ULONGLONG Total = deltaKernelTime + deltaUserTime;

	if (Total == 0) {
		coreUser = 0.0f;
		coreKernel = 0.0f;
		coreTotal = 0.0f;
	}
	else {
		// 코어 사용 백분률 계산
		coreTotal = (float)((double)(Total - deltaIdleTime) / Total * 100.0f);
		coreUser = (float)((double)deltaUserTime / Total * 100.0f);
		coreKernel = (float)((double)(deltaKernelTime - deltaIdleTime) / Total * 100.0f);
	}

	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
	prevIdleTime = curIdleTime;	
}

float MachineCpuMonitor::GetTotalCpuUsage(void) {
	return coreTotal;
}

float MachineCpuMonitor::GetUserCpuUsage(void) {
	return coreUser;
}

float MachineCpuMonitor::GetKernelCpuUsage(void) {
	return coreKernel;
}