#include "CpuUsage.h"

// 확인 프로세스 핸들
CpuUsage::CpuUsage(HANDLE hProcess) {
	if (hProcess == INVALID_HANDLE_VALUE) {
		h_process = GetCurrentProcess();
	}

	// 프로세스 실행률 == (사용시간 / 논리코어 수)
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	NumOfCore = SystemInfo.dwNumberOfProcessors; // 논리 코어 개수
	coreTotal = 0;
	coreUser = 0;
	coreKernel = 0;
	processTotal = 0;
	processUser = 0;
	processKernel = 0;
	prevKernelTime.QuadPart = 0;
	prevUserTime.QuadPart = 0;
	prevIdleTime.QuadPart = 0;
	prevUserTime.QuadPart = 0;
	prevKernelTime.QuadPart = 0;
	prevTime.QuadPart = 0;
	UpdateCpuTime();
}

// CPU 사용률 갱신 (500 ~ 1000 ms 주기 호출)
void CpuUsage::UpdateCpuTime() {
	ULONGLONG deltaTime;
	ULARGE_INTEGER curIdleTime;
	ULARGE_INTEGER curkernelTime;
	ULARGE_INTEGER curUserTime;

	//---------------------------------------------------------
	// 시스템 전체 코어 사용률 계산
	//---------------------------------------------------------

	// 누적 코어 사용 시간 (커널, 유저, idle)
	if (GetSystemTimes((PFILETIME)&curIdleTime, (PFILETIME)&curkernelTime, (PFILETIME)&curUserTime) == false) {
		return;
	}

	// 델타 코어 사용 시간 (현재 - 마지막 호출)
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

	//---------------------------------------------------------
	// 대상 프로세스 코어 사용률 계산
	//---------------------------------------------------------

	ULARGE_INTEGER None; // 미사용 변수
	ULARGE_INTEGER NowTime;

	// 현재 시간
	GetSystemTimeAsFileTime((LPFILETIME)&NowTime);
	// 해당 프로세스 현재까지 코어 사용률
	GetProcessTimes(h_process, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&curkernelTime, (LPFILETIME)&curUserTime);

	// 델타 코어 사용 시간 (해당 프로세스에서의)
	deltaTime = NowTime.QuadPart - prevTime.QuadPart;
	deltaUserTime = curUserTime.QuadPart - prevUserTime.QuadPart;
	deltaKernelTime = curkernelTime.QuadPart - prevKernelTime.QuadPart;
	Total = deltaKernelTime + deltaUserTime;

	// 코어 사용 시간 백분률
	processTotal = (float)(Total / (double)NumOfCore / (double)deltaTime * 100.0f);
	processKernel = (float)(deltaKernelTime / (double)NumOfCore / (double)deltaTime * 100.0f);
	processUser = (float)(deltaUserTime / (double)NumOfCore / (double)deltaTime * 100.0f);

	prevTime = NowTime;
	prevKernelTime = curkernelTime;
	prevUserTime = curUserTime;
}

float CpuUsage::ProcessorTotal(void) {
	return coreTotal;
}

float CpuUsage::ProcessorUser(void) {
	return coreUser;
}

float CpuUsage::ProcessorKernel(void) {
	return coreKernel;
}

float CpuUsage::ProcessTotal(void) {
	return processTotal;
}

float CpuUsage::ProcessUser(void) {
	return processUser;
}

float CpuUsage::ProcessKernel(void) {
	return processKernel;
}